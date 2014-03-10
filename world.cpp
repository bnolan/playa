#include "world.h"
#include <unistd.h>

World::World(){
  broadphase_ = new btDbvtBroadphase();
  collision_configuration_ = new btDefaultCollisionConfiguration();
  dispatcher_ = new btCollisionDispatcher(collision_configuration_);
  solver_ = new btSequentialImpulseConstraintSolver;
  dynamics_world_ = new btDiscreteDynamicsWorld(dispatcher_,broadphase_,solver_,collision_configuration_);
  dynamics_world_->setGravity(btVector3(0,-10,0));
}
  
void World::addGround(){
  btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
  btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0)));
  btRigidBody::btRigidBodyConstructionInfo info(0,groundMotionState,groundShape,btVector3(0,0,0));
  info.m_restitution = 0.9f;
  //info.m_friction = 1.5f;
  btRigidBody* groundRigidBody = new btRigidBody(info);
  dynamics_world_->addRigidBody(groundRigidBody);
}
  
void World::addObject(){
  btCollisionShape* fallShape = new btBoxShape(btVector3(1,1,1));

  btQuaternion quat(0,0,0,1);
  quat.setEuler(1.57,1.57,1.57);

  btDefaultMotionState* fallMotionState =
          new btDefaultMotionState(btTransform(quat,btVector3(0,100,0)));
  btScalar mass = 1.0f;
  btVector3 fallInertia(0,0,0);
  fallShape->calculateLocalInertia(mass,fallInertia);

  btRigidBody::btRigidBodyConstructionInfo info(mass,fallMotionState,fallShape,fallInertia);
  info.m_restitution = 0.9f;
  // info.m_friction = 0.5f;

  std::shared_ptr<btRigidBody> body(new btRigidBody(info));
  dynamics_world_->addRigidBody(body.get());
  objects_.push_back(body);

  // fall_rigid_body_->applyTorqueImpulse(btVector3(1.0f,1.0f,0)); //Just to try and get it to rotate

  // btTransform tr;
  // tr.setIdentity();
  // tr.setOrigin(btVector3(0,100,0));
  // 
  // btQuaternion quat;
  // quat.setEuler(0,1.57,1.57);
  // tr.setRotation(quat);
  // 
  // fall_rigid_body_->setCenterOfMassTransform(tr);
}
  
void World::simulate(){
  // simulate at half realtime.
  dynamics_world_->stepSimulation(1/20.f,10);
  // dynamics_world_->stepSimulation(1/60.f,10);
  
  // 20fps
  usleep(50000);
  
  // btScalar yaw, pitch, roll;
  // btMatrix3x3 mat = btMatrix3x3(trans.getRotation());
  // mat.getEulerYPR(yaw, pitch, roll);
  // 
  // update.rX = yaw;
  // update.rY = pitch;
  // update.rZ = roll;

  // std::cout << "rotation: " << update.rX << "," << update.rY << "," << update.rZ << "," << update.rW << std::endl;
}

std::vector<UpdatePacket> World::update(){
  std::vector<UpdatePacket> result;
  
  for(std::shared_ptr<btRigidBody> object : objects_){
    UpdatePacket update;
  
    btTransform trans;
    object->getMotionState()->getWorldTransform(trans);

    update.id = 0xBEEF;
    update.pX = trans.getOrigin().getX();
    update.pY = trans.getOrigin().getY();
    update.pZ = trans.getOrigin().getZ();

    std::cout << "height: " << trans.getOrigin().getY() << std::endl;

    update.rX = trans.getRotation().getX();
    update.rY = trans.getRotation().getY();
    update.rZ = trans.getRotation().getZ();
    update.rW = trans.getRotation().getW();
    
    result.push_back(update);
  }
  
  return result;
}

void World::reset(){
  for(std::shared_ptr<btRigidBody> object : objects_){
    btTransform tr;
    tr.setIdentity();
    tr.setOrigin(btVector3(0,100,0));
  
    btQuaternion quat;
    quat.setEuler(0,1.57,1.57);
    tr.setRotation(quat);
  
    object->setCenterOfMassTransform(tr);
  }
}