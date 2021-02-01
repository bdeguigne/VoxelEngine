include .env

CFLAGS = -std=c++17  -I$(VULKAN_SDK_PATH)/include -I${HOMEBREW_LIBS_PATH}/include 
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib -lvulkan `pkg-config --static --libs glfw3` 

# create list of all spv files and set as dependency
vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))

TARGET = VoxelEngine

$(TARGET): $(vertObjFiles) $(fragObjFiles)
$(TARGET): *.cpp *.hpp
	g++ $(CFLAGS) -o $(TARGET) *.cpp $(LDFLAGS)

# make shader targets
%.spv: %
	${GLSLC} $< -o $@

.PHONY: test clean

test: VoxelEngine
	./VoxelEngine

clean:
	rm -f VoxelEngine