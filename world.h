#pragma once

#include <list>
#include <iostream>
#include <bullet/btBulletDynamicsCommon.h>

struct UpdatePacket {
  int32_t id;
  float pX,pY,pZ;
  float rX,rY,rZ,rW;
};

class World{

public:
  btBroadphaseInterface* broadphase_;
  btDiscreteDynamicsWorld* dynamics_world_;
  btSequentialImpulseConstraintSolver* solver_;
  btCollisionDispatcher* dispatcher_;
  btDefaultCollisionConfiguration* collision_configuration_;
  // btRigidBody* fall_rigid_body_;

  std::list<btRigidBody*> objects_;
  
  World();
  void addGround();
  void addObject();
  void simulate();
  std::list<UpdatePacket> update();
};
  
