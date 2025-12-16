#include "services/auth_service.h"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <chrono>
#include <vector>
#include <stdexcept>

using json = nlohmann::json;

static std::string base64url_encode(const std::string &in) {
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO *bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);
  BIO_write(b64, in.data(), static_cast<int>(in.size()));
  BIO_flush(b64);
  BUF_MEM *bptr = nullptr;
  BIO_get_mem_ptr(b64, &bptr);
  std::string out(bptr->data, bptr->length);
  BIO_free_all(b64);
  for (auto &c : out) if (c == '+') c = '-'; else if (c == '/') c = '_';
  while (!out.empty() && out.back() == '=') out.pop_back();
  return out;
}

static std::string base64url_decode(const std::string &in) {
  std::string b64 = in;
  for (auto &c : b64) if (c == '-') c = '+'; else if (c == '_') c = '/';
  while (b64.size() % 4) b64.push_back('=');
  BIO *b64bio = BIO_new(BIO_f_base64());
  BIO_set_flags(b64bio, BIO_FLAGS_BASE64_NO_NL);
  BIO *bmem = BIO_new_mem_buf(b64.data(), static_cast<int>(b64.size()));
  bmem = BIO_push(b64bio, bmem);
  std::vector<char> out(b64.size());
  int len = BIO_read(bmem, out.data(), static_cast<int>(out.size()));
  BIO_free_all(bmem);
  if (len <= 0) return {};
  return std::string(out.data(), static_cast<size_t>(len));
}

// file-scoped helper (implementation detail)
static std::string hmac_sha256_base64url_impl(const std::string &key, const std::string &data) {
  unsigned int len = EVP_MAX_MD_SIZE;
  std::vector<unsigned char> digest(len);
  HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()), reinterpret_cast<const unsigned char*>(data.data()), data.size(), digest.data(), &len);
  return base64url_encode(std::string(reinterpret_cast<char*>(digest.data()), len));
}

// Public class method declared in header
std::string AuthService::hmac_sha256_base64url(const std::string &key, const std::string &data) {
  return hmac_sha256_base64url_impl(key, data);
}

AuthService::AuthService(std::string secret, int token_ttl_seconds) : secret_(std::move(secret)), ttl_(token_ttl_seconds) {
  if (secret_.empty()) throw std::invalid_argument("JWT secret required");
}

std::string AuthService::issue_token(const std::string &username, const std::string &role) {
  json header = {{"alg","HS256"}, {"typ","JWT"}};
  using namespace std::chrono;
  auto now = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
  json payload = {{"sub", username}, {"role", role}, {"iat", now}, {"exp", now + ttl_}};
  std::string header_b = base64url_encode(header.dump());
  std::string payload_b = base64url_encode(payload.dump());
  std::string signing_input = header_b + "." + payload_b;
  std::string sig = hmac_sha256_base64url(secret_, signing_input);
  return signing_input + "." + sig;
}

json AuthService::verify_token(const std::string &token) const {
  // split
  size_t p = 0, q = 0;
  std::vector<std::string> parts;
  while ((q = token.find('.', p)) != std::string::npos) { parts.push_back(token.substr(p, q-p)); p = q+1; }
  parts.push_back(token.substr(p));
  if (parts.size() != 3) throw std::runtime_error("invalid token format");
  std::string signing_input = parts[0] + "." + parts[1];
  std::string expected = hmac_sha256_base64url(secret_, signing_input);
  if (expected != parts[2]) throw std::runtime_error("invalid signature");
  std::string payload_raw = base64url_decode(parts[1]);
  if (payload_raw.empty()) throw std::runtime_error("payload decode failed");
  json payload = json::parse(payload_raw);
  if (!payload.contains("exp")) throw std::runtime_error("missing exp");
  using namespace std::chrono;
  auto now = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
  if (payload["exp"].get<long long>() < now) throw std::runtime_error("token expired");
  return payload;
}
