#ifndef TV_CHANNEL_CONTROLLER_H
#define TV_CHANNEL_CONTROLLER_H

#include "TVController.h"
#include <algorithm>
#include <set>
#include <vector>

class TVChannelController : public TVController {
private:
  Tuner &tuner_;
  std::vector<int> buffer;           // 숫자 입력 버퍼
  std::set<int> favorites;           // 즐겨찾기 (정렬 + 중복 제거)
  std::vector<int> searchedChannels; // 채널 검색 결과 (정렬)

  void applyBuffer() {
    if (buffer.empty())
      return;

    int value = 0;

    for (int n : buffer) {
      value = value * 10 + n;
    }

    tuner_.setCH(std::to_string(value));
    buffer.clear();
  }

public:
  explicit TVChannelController(Tuner &tuner)
      : TVController(&tuner), tuner_(tuner) {}
  // 숫자 입력
  void pressNumber(int n) {
    if (n < 0 || n > 9)
      throw std::invalid_argument("invalid number");

    buffer.push_back(n);

    // 3자리 이상 -> 무효화
    if (buffer.size() >= 3) {
      buffer.clear();
      return;
    }

    // 두 자리 자동 변경
    if (buffer.size() == 2) {
      applyBuffer();
    }
  }

  void pressConfirm() { applyBuffer(); }

  void pressOther() { buffer.clear(); }

  // 즐겨찾기 토글
  void pressFavorite() {
    int current = std::stoi(tuner_.getCurrentCH());

    auto it = favorites.find(current);

    if (it == favorites.end()) {
      favorites.insert(current);
    } else {
      favorites.erase(it);
    }
  }

  void addFavorite(int ch) { favorites.insert(ch); }

  const std::vector<int> getFavoriteChannels() const {
    return std::vector<int>(favorites.begin(), favorites.end());
  }

  // 다음 즐겨찾기 이동
  void pressNextFavorite() {
    if (favorites.empty())
      return;

    int current = std::stoi(tuner_.getCurrentCH());

    auto it = favorites.upper_bound(current);

    if (it == favorites.end()) {
      tuner_.setCH(std::to_string(*favorites.begin()));
    } else {
      tuner_.setCH(std::to_string(*it));
    }
  }

  // 채널 검색 (Tuner seekCH로 시청 가능 채널 수집)
  void pressChannelSearch() {
    searchedChannels.clear();
    std::string ch = tuner_.seekCH();
    int firstVal = std::stoi(ch);
    searchedChannels.push_back(firstVal);
    while (true) {
      ch = tuner_.seekCH();
      int val = std::stoi(ch);
      if (val == firstVal)
        break;
      searchedChannels.push_back(val);
    }
    std::sort(searchedChannels.begin(), searchedChannels.end());
  }

  void pressChannelUp() {
    if (searchedChannels.empty()) {
      int ch = std::stoi(tuner_.getCurrentCH());
      ch = (ch + 1) % 100;
      tuner_.setCH(std::to_string(ch));
      return;
    }
    int current = std::stoi(tuner_.getCurrentCH());
    auto it =
        std::upper_bound(searchedChannels.begin(), searchedChannels.end(), current);
    if (it == searchedChannels.end())
      tuner_.setCH(std::to_string(searchedChannels.front()));
    else
      tuner_.setCH(std::to_string(*it));
  }

  void pressChannelDown() {
    if (searchedChannels.empty()) {
      int ch = std::stoi(tuner_.getCurrentCH());
      ch = (ch - 1 + 100) % 100;
      tuner_.setCH(std::to_string(ch));
      return;
    }
    int current = std::stoi(tuner_.getCurrentCH());
    auto it =
        std::lower_bound(searchedChannels.begin(), searchedChannels.end(), current);
    if (it != searchedChannels.end() && *it == current) {
      if (it == searchedChannels.begin())
        tuner_.setCH(std::to_string(searchedChannels.back()));
      else {
        --it;
        tuner_.setCH(std::to_string(*it));
      }
    } else {
      auto it2 =
          std::upper_bound(searchedChannels.begin(), searchedChannels.end(), current);
      if (it2 == searchedChannels.begin())
        tuner_.setCH(std::to_string(searchedChannels.back()));
      else {
        --it2;
        tuner_.setCH(std::to_string(*it2));
      }
    }
  }

  const std::vector<int> &getSearchedChannels() const { return searchedChannels; }
};
#endif // TV_CHANNEL_CONTROLLER_H