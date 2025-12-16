#pragma once

#include <string>
#include <nlohmann/json.hpp>

class AuthService {
public:
  explicit AuthService(std::string secret, int token_ttl_seconds = 3600);
  // issue a JWT for the given subject (username) and role
  std::string issue_token(const std::string &username, const std::string &role);
  // verify token, returns payload if valid
  nlohmann::json verify_token(const std::string &token) const;

private:
  std::string secret_;
  int ttl_{};
  static std::string base64url(const std::string &in);
  static std::string hmac_sha256_base64url(const std::string &key, const std::string &data);
};

