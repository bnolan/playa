#include "world.h"
//#include <unistd.h>
#include <ctime>

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

uint32_t world_id = 0xF00;

class Object{
public:
  unsigned int id_;
  int dead_;
  time_t killed_at_;
  
  Object(){
    id_ = world_id++;
    dead_ = false;
  }
  
  int is_dead(){
    return dead_;
  }
  
  void kill(){
    dead_ = true;
    time(&killed_at_);
  }
  
  int ready_to_reap(){
    time_t now;
    time(&now);
    
    // older than a minute? reap that suckah.
    return killed_at_ - now > 60 * 1000;
  }
};

void World::addObject(){
  btCollisionShape* fallShape = new btBoxShape(btVector3(5,5,5));

  btQuaternion quat(0,0,0,1);
  quat.setEuler(0,0,0);

  btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(quat,btVector3(0,50.0f,0)));
  btScalar mass = 1.0f;
  btVector3 fallInertia(0,0,0);
  fallShape->calculateLocalInertia(mass,fallInertia);

  btRigidBody::btRigidBodyConstructionInfo info(mass,fallMotionState,fallShape,fallInertia);
  info.m_restitution = 0.5f;
  info.m_friction = 0.5f;

  std::shared_ptr<btRigidBody> body(new btRigidBody(info));
  dynamics_world_->addRigidBody(body.get());
  body->setUserPointer(new Object);
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
  dynamics_world_->stepSimulation(1/20.0f,10.0f);
  // dynamics_world_->stepSimulation(1/60.f,10);

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
  
    Object *o = reinterpret_cast<Object *>(object->getUserPointer());
    update.id = o->id_;
    
    if(o->is_dead()){
      update.type = 0x20;
    }else{
      update.type = 0x10;
    }

    btTransform trans;
    object->getMotionState()->getWorldTransform(trans);
    update.pX = trans.getOrigin().getX();
    update.pY = trans.getOrigin().getY();
    update.pZ = trans.getOrigin().getZ();
    update.rX = trans.getRotation().getX();
    update.rY = trans.getRotation().getY();
    update.rZ = trans.getRotation().getZ();
    update.rW = trans.getRotation().getW();

    // uint8_t *buffer = (uint8_t *) &update;
    // for(int i=0;i<sizeof(UpdatePacket);i++){
    //   printf("%02X", buffer[i]);
    // }
    //printf("\n");
    
    result.push_back(update);
  }
  
  return result;
}

void World::reset(){
  for(std::shared_ptr<btRigidBody> obj : objects_){
    dynamics_world_->removeCollisionObject(obj.get());
    
    Object *o = reinterpret_cast<Object *>(obj->getUserPointer());
    o->kill();
  }
  
  // objects_.clear();
        
  // std::cout << "Reseting.. " << object.get() << std::endl;
  // 
  // btTransform tr;
  // tr.setIdentity();
  // tr.setOrigin(btVector3(0,50.0f,0));
  // 
  // btQuaternion quat;
  // quat.setEuler(0,0,0);
  // tr.setRotation(quat);
  // 
  // object->setCenterOfMassTransform(tr);
}
