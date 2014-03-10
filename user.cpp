User::User(int id){
  id_ = id;
  name_ = "parzival";
}
  
std::string User::inspect(){
  return name_ + std::string(" ") + std::to_string(id_);
}

