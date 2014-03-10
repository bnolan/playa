
all:
# 	g++ main.cpp -L./lib/ -I/usr/local/include -I/usr/local/include/bullet -L/usr/local/lib/ -lLinearMath -lBulletMultiThreaded -lBulletCollision -lBulletDynamics -ljson -lcurl -std=c++11
# 	g++ server.cpp world.cpp -L./lib/ -I/usr/local/include -I/usr/local/include/bullet -L/usr/local/lib/ -lwebsockets -llua -lLinearMath -lBulletMultiThreaded -lBulletCollision -lBulletDynamics -ljson -std=c++11
	g++ server.cpp user.cpp world.cpp -L./lib/ -I/usr/local/include -I/usr/local/include/bullet -L/usr/local/lib/ -lwebsockets -llua -lLinearMath -lBulletMultiThreaded -lBulletCollision -lBulletDynamics -std=c++11
#  -I./build/bullet/src/
