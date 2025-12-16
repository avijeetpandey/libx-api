#include <gtest/gtest.h>
#include "services/auth_service.h"

TEST(AuthService, IssueAndVerify) {
  AuthService s("test-secret", 3600);
  auto tok = s.issue_token("alice", "admin");
  ASSERT_FALSE(tok.empty());
  auto payload = s.verify_token(tok);
  EXPECT_EQ(payload["sub"].get<std::string>(), "alice");
  EXPECT_EQ(payload["role"].get<std::string>(), "admin");
}
TEST(AuthService, ExpiredToken) {
  // ttl negative to create already expired token
  AuthService s("test-secret", -10);
  auto tok = s.issue_token("bob", "user");
  ASSERT_FALSE(tok.empty());
  EXPECT_THROW(s.verify_token(tok), std::runtime_error);
}
