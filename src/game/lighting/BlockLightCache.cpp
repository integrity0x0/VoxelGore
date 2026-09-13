#include "BlockLightCache.h"

namespace gm::lighting {

const BlockLightData* BlockLightCache::Require(uint32_t blockId) {
  if (const auto* data = Get(blockId)) {
    return data;
  }

  const Block* block = blockManager_->block(blockId);

  if (!block || !block->light()) {
    Set(blockId, {
                     .id = kInvalidChannelId,
                     .strength = 0,
                     .passingLight = true,
                 });

    return Get(blockId);
  }

  const auto& light = *block->light();

  const ChannelId channelId = registry_->Require(light.channelId);

  if (channelId == kInvalidChannelId) {
    Set(blockId, {
                     .id = kInvalidChannelId,
                     .strength = 0,
                 });

    return Get(blockId);
  }

  Set(blockId, {
                   .id = channelId,
                   .strength = light.strength,
               });

  return Get(blockId);
}

}  // namespace gm::lighting