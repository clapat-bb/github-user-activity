#include "../include/activity.hpp"

#include <cstddef>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>
#include <iterator>
#include <json.hpp>
#include <new>
#include <stdatomic.h>
#include <stdexcept>
#include <string>
#include <vector>

void from_json(const nlohmann::json &j, Repo &r) {
  j.at("name").get_to(r.name);
}

void from_json(const nlohmann::json &j, GithubActivity &a) {
  j.at("type").get_to(a.type);
  j.at("repo").get_to(a.repo);
  j.at("create_at").get_to(a.creatAt);
  j.at("payload").at("action").get_to(a.payload.action);
  j.at("payload").at("ref").get_to(a.payload.ref);
  j.at("payload").at("ref_type").get_to(a.payload.refType);

  if (j.at("payload").contains("commits") &&
      j.at("payload")["commits"].is_array()) {
    for (const auto &commit_json : j.at("payload")["commits"]) {
      Commit commit;
      commit_json.at("message").get_to(commit.message);
      a.payload.commits.push_back(commit);
    }
  }
}

size_t WriteCallBack(void *contents, size_t size, size_t nmemb,
                     std::string *s) {
  size_t newLength = size * nmemb;
  try {
    s->append((char *)contents, newLength);
  } catch (std::bad_alloc &e) {
    return 0;
  }
  return newLength;
}

std::vector<GithubActivity> fetchGithubActivity(const std::string &username) {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl = curl_easy_init();
  if (curl) {
    std::string url = "https://api.github.com/users/" + username + "/events";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
      throw std::runtime_error("curl_easy_perform() failed: " +
                               std::string(curl_easy_strerror(res)));
    }
    nlohmann::json jsonData = nlohmann::json::parse(readBuffer);
    return jsonData.get<std::vector<GithubActivity>>();
  }
  return {};
}

void displayActivity(const std::string &username,
                     const std::vector<GithubActivity> &events) {
  if (events.empty()) {
    std::cerr << "No activity found" << '\n';
    return;
  }

  std::cout << username << "'s recent activity(s)\n";
  for (const auto &event : events) {
    std::string action;
    if (event.type == "PushEvent") {
      action = "Pushed " + std::to_string(event.payload.commits.size()) +
               " commit(s) to " + event.repo.name;

    } else if (event.type == "IssuesEvent") {
      action = event.payload.action + " an issue in " + event.repo.name;
    } else if (event.type == "WatchEvent") {
      action = "Starred " + event.repo.name;
    } else if (event.type == "ForkEvent") {
      action = "Forked " + event.repo.name;
    } else if (event.type == "CreateEvent") {
      action = "Create " + event.payload.refType + " in " + event.repo.name;
    } else {
      action = event.type + " in " + event.repo.name;
    }
    std::cout << "- " << action << '\n';
  }
}
