#include "ChannelRegistry.h"

#include "ChannelParser.h"
#include "../../core/PathPrefixes.h"

namespace gm::lighting {

namespace {
constexpr std::string_view kChannelPath = "light_channels/";
constexpr std::string_view kChannelExtension = ".json";
}  // namespace

ChannelRegistry::ChannelRegistry(const ChannelDefinition& sunDefinition) {
  std::ignore = Register(sunDefinition);
}

ChannelId ChannelRegistry::Register(ChannelDefinition definition) {
  if (const auto it = ids_.find(definition.name); it != ids_.end()) {
    definitions_[it->second] = std::make_unique<ChannelDefinition>(std::move(definition));
    return it->second;
  }

  const auto id = static_cast<ChannelId>(definitions_.size());

  ids_.emplace(definition.name, id);
  definitions_.emplace_back(std::make_unique<ChannelDefinition>(std::move(definition)));

  return id;
}

ChannelId ChannelRegistry::Find(std::string_view name) const {
  auto it = ids_.find(name);
  return it != ids_.end() ? it->second : kInvalidChannelId;
}

ChannelId ChannelRegistry::Require(std::string_view name) {
  if (auto channelId = Find(name); channelId != kInvalidChannelId) {
    return channelId;
  }

  std::string path;
  path.reserve(core::kAssetsPrefix.size() + kChannelPath.size() + name.size() +
               kChannelExtension.size());
  path += core::kAssetsPrefix;
  path += kChannelPath;
  path += name;
  path += kChannelExtension;

  auto definition = ChannelParser::Parse(path);
  if (!definition) {
    return kInvalidChannelId;
  }
  
  const ChannelId channelId = static_cast<ChannelId>(definitions_.size());

  ids_.emplace(definition->name, channelId);
  definitions_.emplace_back(std::make_unique<ChannelDefinition>(std::move(*definition)));

  return channelId;
}

const ChannelDefinition* ChannelRegistry::GetById(ChannelId id) const {
  if (id >= definitions_.size()) {
    return nullptr;
  }

  return definitions_[id].get();
}

const ChannelDefinition* ChannelRegistry::GetByName(std::string_view name) const { 
  return GetById(Find(name));
}
}  // namespace gm::lighting