#include "../include/activity.hpp"
#include <exception>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "pls provide a username" << std::endl;
    return 1;
  }

  std::string username = argv[1];
  try {
    std::vector<GithubActivity> activities = fetchGithubActivity(username);
    displayActivity(username, activities);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
  return 0;
}
