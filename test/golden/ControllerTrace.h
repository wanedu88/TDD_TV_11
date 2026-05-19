#pragma once

#include "TVChannelController.h"
#include "fakeTuner.h"
#include "Tuner.h"
#include <sstream>
#include <string>
#include <vector>

// TextTest-style transcript: actions (>) and observable state snapshots.
class ControllerTrace {
public:
  ControllerTrace(TVChannelController &ctrl, Tuner &tuner)
      : ctrl_(ctrl), tuner_(tuner) {}

  void header(const std::string &line) { out_ << "# " << line << "\n"; }

  void pressNumber(int n) {
    out_ << "> pressNumber(" << n << ")\n";
    ctrl_.pressNumber(n);
    snapshot();
  }

  void pressConfirm() {
    out_ << "> pressConfirm()\n";
    ctrl_.pressConfirm();
    snapshot();
  }

  void pressOther() {
    out_ << "> pressOther()\n";
    ctrl_.pressOther();
    snapshot();
  }

  void pressFavorite() {
    out_ << "> pressFavorite()\n";
    ctrl_.pressFavorite();
    snapshotFavorites();
  }

  void addFavorite(int ch) {
    out_ << "> addFavorite(" << ch << ")\n";
    ctrl_.addFavorite(ch);
    snapshotFavorites();
  }

  void pressNextFavorite() {
    out_ << "> pressNextFavorite()\n";
    ctrl_.pressNextFavorite();
    snapshot();
  }

  void pressChannelSearch() {
    out_ << "> pressChannelSearch()\n";
    ctrl_.pressChannelSearch();
    snapshotSearch();
  }

  void pressChannelUp() {
    out_ << "> pressChannelUp()\n";
    ctrl_.pressChannelUp();
    snapshot();
  }

  void pressChannelDown() {
    out_ << "> pressChannelDown()\n";
    ctrl_.pressChannelDown();
    snapshot();
  }

  void setChannel(int ch) {
    out_ << "> setCH(" << ch << ")\n";
    tuner_.setCH(std::to_string(ch));
    snapshot();
  }

  std::string str() const { return out_.str(); }

private:
  static std::string joinInts(const std::vector<int> &values) {
    std::ostringstream ss;
    ss << "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
      if (i > 0) {
        ss << ",";
      }
      ss << values[i];
    }
    ss << "]";
    return ss.str();
  }

  void snapshot() { out_ << "CH=" << tuner_.getCurrentCH() << "\n"; }

  void snapshotFavorites() {
    snapshot();
    out_ << "favorites=" << joinInts(ctrl_.getFavoriteChannels()) << "\n";
  }

  void snapshotSearch() {
    snapshot();
    out_ << "searched=" << joinInts(ctrl_.getSearchedChannels()) << "\n";
  }

  TVChannelController &ctrl_;
  Tuner &tuner_;
  std::ostringstream out_;
};
