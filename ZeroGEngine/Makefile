CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -I/usr/include/nlohmann -I/usr/local/PhysX/include -DNDEBUG

CPP_SRC = src/ECS.cpp src/Entity.cpp src/Mesh.cpp src/PhysicsSystem.cpp src/RenderSystem.cpp src/Scene.cpp
C_SRC = src/glad.c
OBJ = $(CPP_SRC:.cpp=.o) $(C_SRC:.c=.o)


LDFLAGS = -lPhysX -lPhysXCommon -lPhysXCooking -lPhysXFoundation


libZeroGEngine.a: $(OBJ)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) libZeroGEngine.a
