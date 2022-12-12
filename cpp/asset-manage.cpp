#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "t9/hash.h"
#include "t9/reference_count.h"

struct AssetId {
  std::size_t index;
  std::uint32_t revision;
};

class AssetOwner {
 public:
  virtual ~AssetOwner() = default;
  virtual void on_drop_asset(AssetId id) = 0;
};

class Asset : public t9::RcObject {
 public:
  enum class State {
    INIT,
    LOADED,
    READY,
    NOT_FOUND,
    INVALID,
  };

  struct InitParam {
    AssetId id;
    std::string_view path;
    std::uint32_t path_hash = 0;
    AssetOwner* owner = nullptr;
  };

 private:
  AssetId id_;
  std::string path_;
  std::uint32_t path_hash_ = 0;
  std::vector<char> data_;
  AssetOwner* owner_ = nullptr;
  State state_ = State::INIT;

 public:
  explicit Asset(const InitParam& param)
      : id_(param.id),
        path_(param.path),
        path_hash_(param.path_hash),
        owner_(param.owner) {}

  void load() {
    std::ifstream in(path_, std::ios::binary);
    if (!in.is_open()) {
      state_ = State::NOT_FOUND;
      return;
    }

    in.seekg(0, std::ios_base::end);
    auto size = in.tellg();
    in.seekg(0, std::ios_base::beg);

    data_.resize(size);
    in.read(data_.data(), size);

    state_ = on_load() ? State::LOADED : State::INVALID;
  }

  void postprocess() {
    if (state_ == State::LOADED) {
      state_ = on_postprocess() ? State::READY : State::INVALID;
    }
  }

  std::string_view path() const { return path_; }
  std::uint32_t path_hash() const { return path_hash_; }
  AssetId id() const { return id_; }
  const char* data() const { return data_.data(); }
  std::size_t data_size() const { return data_.size(); }

  bool is_ready() const { return state_ == State::READY; }

 protected:
  bool on_load() { return true; }
  bool on_postprocess() { return true; }

 protected:
  // t9::RcObject.
  virtual void on_drop() override {
    if (owner_) {
      owner_->on_drop_asset(id_);
    }
  }
};

class AssetManager : private AssetOwner {
 private:
  struct AssetPath {
    std::string_view path;
    std::uint32_t hash = 0;
  };
  struct AssetPathComp {
    bool operator()(AssetPath lhs, AssetPath rhs) const {
      if (lhs.hash < rhs.hash) return true;
      if (lhs.hash > rhs.hash) return false;
      return lhs.path < rhs.path;
    }
  };
  struct AssetStorage {
    std::uint32_t revision = 0;
    std::unique_ptr<Asset> asset;
  };

  using StorageDeque = std::deque<AssetStorage>;
  using IndexDeque = std::deque<std::size_t>;
  using IndexMap = std::map<AssetPath, std::size_t, AssetPathComp>;

 private:
  StorageDeque assets_;
  IndexDeque deleted_indices_;
  IndexMap indices_;
  IndexDeque delete_requests_;

 public:
  t9::Rc<Asset> load(std::string_view path) {
    auto hash = t9::fxhash(path.data(), path.length());
    if (auto asset = find_(path, hash)) {
      return t9::Rc<Asset>(asset, true);
    }
    std::size_t index = 0;
    if (deleted_indices_.empty()) {
      index = assets_.size();
      assets_.emplace_back().revision = 1u;
    } else {
      index = deleted_indices_.front();
      deleted_indices_.pop_front();
    }
    Asset::InitParam param{
        AssetId{index, assets_[index].revision},
        path,
        hash,
        this,
    };
    auto asset = new Asset(param);
    assets_[index].asset.reset(asset);
    indices_.emplace(AssetPath{asset->path(), hash}, index);
    asset->load();
    asset->postprocess();
    return t9::Rc<Asset>(asset, false);
  }
  t9::Rc<Asset> find(std::string_view path) {
    auto hash = t9::fxhash(path.data(), path.length());
    if (auto asset = find_(path, hash)) {
      return t9::Rc<Asset>(asset, true);
    }
    return nullptr;
  }

  void proccess_delete_requests() {
    for (auto index : delete_requests_) {
      if (!assets_[index].asset) continue;
      if (assets_[index].asset->reference_count() > 0) continue;
      remove_(index);
    }
    delete_requests_.clear();
  }

 private:
  Asset* find_(std::string_view path, std::uint32_t hash) {
    auto it = indices_.find(AssetPath{path, hash});
    if (it == indices_.end()) return nullptr;
    return assets_[it->second].asset.get();
  }
  void remove_(std::size_t index) {
    auto& storage = assets_[index];
    auto asset = storage.asset.get();
    indices_.erase(AssetPath{asset->path(), asset->path_hash()});
    deleted_indices_.push_back(index);
    storage.revision = std::max(storage.revision + 1, 1u);
    storage.asset.reset();
  }

 private:
  virtual void on_drop_asset(AssetId id) override {
    delete_requests_.emplace_back(id.index);
  }
};

int main() {
  AssetManager assets;
  {
    auto a = assets.load("til.sln");
    if (a->is_ready()) {
      std::cout << a->path() << " is ready." << std::endl;
    }
  }
  {
    auto a = assets.load("til.sln");
    if (a->is_ready()) {
      std::cout << a->path() << " is ready." << std::endl;
    }
  }
  assets.proccess_delete_requests();
}
