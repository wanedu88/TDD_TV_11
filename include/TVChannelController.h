#ifndef TV_CHANNEL_CONTROLLER_H
#define TV_CHANNEL_CONTROLLER_H

#include "TVController.h"
#include <algorithm>
#include <set>
#include <vector>

class TVChannelController : public TVController {
private:
  Tuner &tuner_;
  std::vector<int> buffer; // 숫자 입력 버퍼
  std::set<int> favorites; // 즐겨찾기 (정렬 + 중복 제거)

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

  void pressOther() {
    buffer.clear();
    tuner_.setCH("0");
  }

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
};
#endif // TV_CHANNEL_CONTROLLER_H