CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra \
           -fsanitize=address \
           -fno-omit-frame-pointer \
           -g \
           -I../ZeroGEngine/include \
           -I/usr/include/nlohmann \
           -Iinclude \
           -I/usr/local/PhysX/include \
           -DNDEBUG

LDFLAGS = -fsanitize=address \
          -L../ZeroGEngine \
          -lZeroGEngine \
          -lPhysXFoundation \
          -lPhysXCommon \
          -lPhysX \
          -lPhysXCooking \
          -lglfw -ldl -lpthread

SRC_DIR = src
OBJ_DIR = obj

SRC_CPP = $(wildcard $(SRC_DIR)/*.cpp)
SRC_C   = $(wildcard $(SRC_DIR)/*.c)

OBJ_CPP = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_CPP))
OBJ_C   = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_C))

OBJ = $(OBJ_CPP) $(OBJ_C)

runtime: $(OBJ)
	$(CXX) $(OBJ) -o runtime $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) runtime
