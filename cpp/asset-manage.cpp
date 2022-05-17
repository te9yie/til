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
  enum class ReturnCode {
    OK,
    NOT_FOUND,
  };

 private:
  std::string path_;
  std::uint32_t path_hash_ = 0;
  AssetId id_;
  std::vector<char> data_;
  AssetOwner* owner_ = nullptr;

 public:
  Asset() = default;

  ReturnCode load() {
    std::ifstream in(path_, std::ios::binary);
    if (!in.is_open()) {
      return ReturnCode::NOT_FOUND;
    }

    in.seekg(0, std::ios_base::end);
    auto size = in.tellg();
    in.seekg(0, std::ios_base::beg);

    data_.resize(size);
    in.read(data_.data(), size);

    return ReturnCode::OK;
  }

  std::string_view path() const { return path_; }
  std::uint32_t path_hash() const { return path_hash_; }
  AssetId id() const { return id_; }

 protected:
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

 public:
  t9::Rc<Asset> load(std::string_view path) {
    auto hash = t9::make_string_hash(path);
    if (auto asset = find_(path, hash)) {
      return t9::Rc<Asset>(asset, true);
    }
    return nullptr;
  }
  t9::Rc<Asset> find(std::string_view path) {
    auto hash = t9::make_string_hash(path);
    if (auto asset = find_(path, hash)) {
      return t9::Rc<Asset>(asset, true);
    }
    return nullptr;
  }

 private:
  Asset* find_(std::string_view path, std::uint32_t hash) {
    auto it = indices_.find(AssetPath{path, hash});
    if (it == indices_.end()) return nullptr;
    return assets_[it->second].asset.get();
  }

 private:
  virtual void on_drop_asset(AssetId id) override {
    deleted_indices_.emplace_back(id.index);
  }
};

int main() {
  std::cout << "Hello" << std::endl;
  std::cout << "Hello" << std::endl;
}
