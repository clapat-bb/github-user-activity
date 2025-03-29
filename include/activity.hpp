#pragma once

#include <json.hpp>
#include <string>
#include <vector>

struct Repo {
  std::string name;
};

struct Commit {
  std::string message;
};

struct GithubActivity {
  std::string type;
  Repo repo;
  std::string createdAt;
  struct {
    std::string action;
    std::string ref;
    std::string refType;
    std::vector<Commit> commits;
  } payload;
};

void from_json(const nlohmann::json &j, Repo &r);
void from_json(const nlohmann::json &j, GithubActivity &a);

std::vector<GithubActivity> fetchGithubActivity(const std::string &username);

void displayActivity(const std::string &username,
                     const std::vector<GithubActivity> &events);
