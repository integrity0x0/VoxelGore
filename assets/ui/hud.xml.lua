local picked = 0
local pickedSlot = nil

local function IndexOf(node)
  print("[IndexOf] start, node=", tostring(node))
  local parent = node:GetParent()
  print("[IndexOf] parent=", tostring(parent))
  local children = parent:GetChildren()
  print("[IndexOf] children count=", #children)
  for i, child in ipairs(children) do
    print("[IndexOf] iter i=", i, "child=", tostring(child), "equals=", child == node)
    if child == node then
      print("[IndexOf] found at i=", i)
      return i
    end
  end
  print("[IndexOf] not found, returning nil")
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

function OnInventorySlotClick(node)
  local id = IndexOf(node)
  if id > inventory.BlockCount() then return end

  if picked == id then
    picked, pickedSlot = 0, nil
  else
    picked, pickedSlot = id, node
  end
end

function OnHotbarSlotClick(node)
  local i = IndexOf(node)
  if picked ~= 0 then
    inventory.SetHotbarBlock(i, picked)
    SetSlot(node, picked)
    pickedSlot:SetChecked(false)
    print("[after uncheck] checked =", pickedSlot:IsChecked())
    picked, pickedSlot = 0, nil
  end
  inventory.Select(i)
end