function f()
  print "called f()"
end

-- f()

World = {}

World.__index = World

function World.create()
   local self = {}         
   setmetatable(self,World)
   -- initializers here..
   return self
end

function World:reset()
  -- print "resetting the world..."
  lua_world_reset()
end

function World:addChild()
  -- print "resetting the world..."
  lua_world_add_child()
end


world = World.create()
