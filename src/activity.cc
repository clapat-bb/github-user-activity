#include "../include/activity.hpp"

#include <cstddef>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/urlapi.h>
#include <iostream>
#include <iterator>
#include <json.hpp>
#include <new>
#include <stdatomic.h>
#include <stdexcept>
#include <string>
#include <vector>

void from_json(const nlohmann::json &j, Repo &r) {
  if (!j.at("name").is_null()) {
    j.at("name").get_to(r.name);
  } else {
    r.name = "N/A";
  }
}

void from_json(const nlohmann::json &j, GithubActivity &a) {
  if (!j.at("type").is_null()) {
    j.at("type").get_to(a.type);
  } else {
    a.type = "N/A";
  }

  from_json(j.at("repo"), a.repo);

  if (!j.at("created_at").is_null()) {
    j.at("created_at").get_to(a.createdAt);
  } else {
    a.createdAt = "N/A";
  }

  if (j.at("payload").contains("action") &&
      !j.at("payload").at("action").is_null()) {
    j.at("payload").at("action").get_to(a.payload.action);
  } else {
    a.payload.action = "N/A";
  }

  if (j.at("payload").contains("ref") && !j.at("payload").at("ref").is_null()) {
    j.at("payload").at("ref").get_to(a.payload.ref);
  } else {
    a.payload.ref = "N/A";
  }

  if (j.at("payload").contains("ref_type") &&
      !j.at("payload").at("ref_type").is_null()) {
    j.at("payload").at("ref_type").get_to(a.payload.refType);
  } else {
    a.payload.refType = "N/A";
  }

  if (j.at("payload").contains("commits") &&
      j.at("payload")["commits"].is_array()) {
    for (const auto &commit_json : j.at("payload")["commits"]) {
      Commit commit;
      if (!commit_json.at("message").is_null()) {
        commit_json.at("message").get_to(commit.message);
      } else {
        commit.message = "N/A";
      }
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
  long http_code = 0;

  curl = curl_easy_init();
  if (curl) {
    std::string url = "https://api.github.com/users/" + username + "/events";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // update;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: MyGitHubActivityApp");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
      throw std::runtime_error("curl_easy_perform() failed: " +
                               std::string(curl_easy_strerror(res)));
    }

    if (http_code != 200) {
      throw std::runtime_error("HTTP request failled with status code: " +
                               std::to_string(http_code));
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
