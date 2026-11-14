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

function OnHotbarSlotClick(node)
  local i = IndexOf(node)
  local picked = inventory.GetPicked()

  if picked ~= 0 then
    inventory.SetHotbarBlock(i, picked)
    SetSlot(node, picked)
    inventory.ClearPicked()

    local invGrid = ui.FindById("inventory")
    if invGrid then invGrid:ClearSelection() end
  end

  inventory.Select(i)
end

function OnInventoryOpen(node)
  if ui.IsPushed("ui/inventory.xml") then
    ui.Pop()
  else
    ui.Push("ui/inventory.xml")
  end
end