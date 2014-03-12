#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <bullet/btBulletDynamicsCommon.h>
#include <stdint.h>


struct UpdatePacket {
  uint32_t id;
  uint8_t type;
  uint8_t family;
  uint8_t instruction;
  uint8_t parameter;
  float pX,pY,pZ;
  float rX,rY,rZ,rW;
} __attribute__((packed));

class World{

public:
  btBroadphaseInterface* broadphase_;
  btDiscreteDynamicsWorld* dynamics_world_;
  btSequentialImpulseConstraintSolver* solver_;
  btCollisionDispatcher* dispatcher_;
  btDefaultCollisionConfiguration* collision_configuration_;

  std::vector<std::shared_ptr<btRigidBody>> objects_;
  // std::vector<shared_ptr<User>> users_;
  
  World();
  void addGround();
  void addObject();
  void simulate();
  void reset();
  std::vector<UpdatePacket> update();
};
  
