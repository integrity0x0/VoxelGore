#include "ChannelRegistry.h"

#include "ChannelParser.h"
#include "../../core/PathPrefixes.h"

namespace gm::lighting {

namespace {
constexpr std::string_view kChannelPath = "light_channels/";
constexpr std::string_view kChannelExtension = ".json";
}  // namespace

ChannelRegistry::ChannelRegistry(ChannelDefinition&& sunDefinition) {
  std::ignore = Register(std::move(sunDefinition));
}

ChannelId ChannelRegistry::Register(ChannelDefinition definition) {
  if (const auto it = ids_.find(definition.id); it != ids_.end()) {
    definitions_[it->second] = std::move(definition);
    return it->second;
  }

  const auto id = static_cast<ChannelId>(definitions_.size());

  ids_.emplace(definition.id, id);
  definitions_.push_back(std::move(definition));

  return id;
}

ChannelId ChannelRegistry::Find(std::string_view id) const {
  auto it = ids_.find(id);
  return it != ids_.end() ? it->second : kInvalidChannelId;
}

ChannelId ChannelRegistry::Require(std::string_view id) {
  if (auto channelId = Find(id); channelId != kInvalidChannelId) {
    return channelId;
  }

  std::string path;
  path.reserve(core::kAssetsPrefix.size() + kChannelPath.size() + id.size() +
               kChannelExtension.size());
  path += core::kAssetsPrefix;
  path += kChannelPath;
  path += id;
  path += kChannelExtension;

  auto definition = ChannelParser::Parse(path);
  if (!definition) {
    return kInvalidChannelId;
  }
  
  const ChannelId channelId = static_cast<ChannelId>(definitions_.size());

  ids_.emplace(definition->id, channelId);
  definitions_.push_back(std::move(*definition));

  return channelId;
}

const ChannelDefinition* ChannelRegistry::Get(ChannelId id) const {
  if (id >= definitions_.size()) {
    return nullptr;
  }

  return &definitions_[id];
}

const ChannelDefinition* ChannelRegistry::Get(std::string_view key) const { 
  return Get(Find(key)); 
}
}  // namespace gm::lighting