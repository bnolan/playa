-- Sandboxing...

-- arg=nil
-- debug.debug=nil
-- debug.getfenv=getfenv
-- debug.getregistry=nil
-- dofile=nil
-- io={write=io.write}
-- loadfile=nil
-- os.execute=nil
-- os.getenv=nil
-- os.remove=nil
-- os.rename=nil
-- os.tmpname=nil
-- package.loaded.io=io
-- package.loaded.package=nil
-- package=nil
-- require=nil

-- function f()
--   print "called f()"
-- end

-- f()

-- World class and singleton...
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

function World:addChild(o)
  lua_world_add_child(o)
end

world = World.create()

Vector = {}
Vector.__index = Vector
function Vector.create(x,y,z)
   local self = {}         
   setmetatable(self,Vector)
   self.x = x
   self.y = y
   self.z = z
   -- initializers here..
   return self
end

-- Vector = {}
-- Vector.__index = Vector
--  
-- function Vector.__add(a, b)
--   if type(a) == "number" then
--     return Vector.new(b.x + a, b.y + a)
--   elseif type(b) == "number" then
--     return Vector.new(a.x + b, a.y + b)
--   else
--     return Vector.new(a.x + b.x, a.y + b.y)
--   end
-- end
--  
-- function Vector.__sub(a, b)
--   if type(a) == "number" then
--     return Vector.new(b.x - a, b.y - a)
--   elseif type(b) == "number" then
--     return Vector.new(a.x - b, a.y - b)
--   else
--     return Vector.new(a.x - b.x, a.y - b.y)
--   end
-- end
--  
-- function Vector.__mul(a, b)
--   if type(a) == "number" then
--     return Vector.new(b.x * a, b.y * a)
--   elseif type(b) == "number" then
--     return Vector.new(a.x * b, a.y * b)
--   else
--     return Vector.new(a.x * b.x, a.y * b.y)
--   end
-- end
--  
-- function Vector.__div(a, b)
--   if type(a) == "number" then
--     return Vector.new(b.x / a, b.y / a)
--   elseif type(b) == "number" then
--     return Vector.new(a.x / b, a.y / b)
--   else
--     return Vector.new(a.x / b.x, a.y / b.y)
--   end
-- end
--  
-- function Vector.__eq(a, b)
--   return a.x == b.x and a.y == b.y
-- end
--  
-- function Vector.__lt(a, b)
--   return a.x < b.x or (a.x == b.x and a.y < b.y)
-- end
--  
-- function Vector.__le(a, b)
--   return a.x <= b.x and a.y <= b.y
-- end
--  
-- function Vector.__tostring(a)
--   return "(" .. a.x .. ", " .. a.y .. ")"
-- end
--  
-- function Vector.new(x, y)
--   return setmetatable({ x = x or 0, y = y or 0 }, Vector)
-- end
--  
-- function Vector.distance(a, b)
--   return (b - a):len()
-- end
--  
-- function Vector:clone()
--   return Vector.new(self.x, self.y)
-- end
--  
-- function Vector:unpack()
--   return self.x, self.y
-- end
--  
-- function Vector:len()
--   return math.sqrt(self.x * self.x + self.y * self.y)
-- end
--  
-- function Vector:lenSq()
--   return self.x * self.x + self.y * self.y
-- end
--  
-- function Vector:normalize()
--   local len = self:len()
--   self.x = self.x / len
--   self.y = self.y / len
--   return self
-- end
--  
-- function Vector:normalized()
--   return self / self:len()
-- end
--  
-- function Vector:rotate(phi)
--   local c = math.cos(phi)
--   local s = math.sin(phi)
--   self.x = c * self.x - s * self.y
--   self.y = s * self.x + c * self.y
--   return self
-- end
--  
-- function Vector:rotated(phi)
--   return self:clone():rotate(phi)
-- end
--  
-- function Vector:perpendicular()
--   return Vector.new(-self.y, self.x)
-- end
--  
-- function Vector:projectOn(other)
--   return (self * other) * other / other:lenSq()
-- end
--  
-- function Vector:cross(other)
--   return self.x * other.y - self.y * other.x
-- end
--  
-- setmetatable(Vector, { __call = function(_, ...) return Vector.new(...) end })


Item = {}
Item.__index = Item
function Item.create()
   local self = {}         
   setmetatable(self,Item)
   self.position_x = 0 --  = Vector.create()
   self.position_y = 0
   self.position_z = 0
   -- initializers here..
   return self
end

function Item:setPosition(p)
  print "eh?"
  print(p)
  self.position_x = p.x
  self.position_y = p.y
  self.position_z = p.z
  print "ehx?"
end  

v = Vector.new(34,56,78)
print(v.x)
print(v)

obj = Item.create()
print(v)
obj.setPosition(v)
lua_world_add_child(obj)

print 'hola!'
