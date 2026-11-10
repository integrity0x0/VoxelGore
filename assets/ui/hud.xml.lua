local picked = 0
local pickedSlot = nil

local function IndexOf(node)
  for i, child in ipairs(node:GetParent():GetChildren()) do
    if child == node then return i end
  end
end

local function SetSlot(slot, blockId)
  local icon, count = table.unpack(slot:GetChildren())
  icon:SetVisible(blockId ~= 0)
  if blockId ~= 0 then icon:SetSrc("@blocks." .. blockId .. ".preview") end
  count:SetVisible(false)
end

function OnInventorySlotClick(node)
  --if pickedSlot then pickedSlot:SetChecked(false) end
  local children = node:GetChildren()
  local icon, count = children[1], children[2]
  --node:SetChecked(true)
  print("slot onclick")
end

function OnHotbarSlotClick(node)
  local i = IndexOf(node)
  if picked ~= 0 then
    inventory.SetHotbarBlock(i, picked)
    SetSlot(node, picked)
    --pickedSlot:SetChecked(false)
    picked, pickedSlot = 0, nil
  end
  inventory.Select(i)
  print("onclick")
end