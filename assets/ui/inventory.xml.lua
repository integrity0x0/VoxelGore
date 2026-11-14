local function IndexOf(node)
  local parent = node:GetParent()
  local children = parent:GetChildren()
  for i, child in ipairs(children) do
    if child == node then return i end
  end
end

local function SetSlot(slot, blockId)
  local children = slot:GetChildren()
  local icon, count = children[1], children[2]

  icon:SetVisible(blockId ~= 0)
  if blockId ~= 0 then
    icon:SetSrc(ui.TextureRegion("@blocks." .. blockId .. ".preview"))
  end
  count:SetVisible(false)
end

function OnOpen()
  local grid = ui.FindById("inventory")
  local blockCount = inventory.BlockCount()

  for i, slot in ipairs(grid:GetChildren()) do
    SetSlot(slot, i <= blockCount and i or 0)
  end
end

function OnInventorySlotClick(node)
  local id = IndexOf(node)
  if id > inventory.BlockCount() then return end

  if node:IsChecked() then
    inventory.ClearPicked()
  else
    inventory.SetPicked(id)
  end
end

function OnClose()
  inventory.ClearPicked()
end