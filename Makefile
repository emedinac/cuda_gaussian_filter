#
# Macros
#
IMG_LDFLAG	= -lpng
LDFLAGS 	= $(IMG_LDFLAG) -lm -L$(LIB_DIR)
INCLUDES    = -I$(SRC_DIR) -I$(LIB_DIR)

CC		    = nvcc
CFLAGS		= -gencode arch=compute_61,code=sm_61 \
		      -gencode arch=compute_52,code=sm_52 \
		      --fmad=false \
		      -O2 -std=c++11 \
		      --compiler-options -Wall

BIN_DIR     = bin
OBJ_DIR     = obj
SRC_DIR     = src
LIB_DIR     = lib

CPP_SRCS	= $(SRC_DIR)/kernel.cpp $(SRC_DIR)/image.cpp
CU_SRCS		= $(SRC_DIR)/main.cu $(SRC_DIR)/gpu_convolution.cu

CPP_OBJS	= $(CPP_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
CU_OBJS		= $(CU_SRCS:$(SRC_DIR)/%.cu=$(OBJ_DIR)/%.o)
TARGET		= $(BIN_DIR)/kernel_convolution

CPP_DEPS	= $(CPP_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.d)
CU_DEPS		= $(CU_SRCS:$(SRC_DIR)/%.cu=$(OBJ_DIR)/%.d)

#
# Rules to compile .o files from .cpp and .cu
#
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cu
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

#
# Rules to generate .d dependency files
#
$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp
	$(CC) -M $(INCLUDES) $< > $@

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cu
	$(CC) -M $(INCLUDES) $< > $@

#
# Generating the target
#
all: $(BIN_DIR) $(OBJ_DIR) $(TARGET)

#
# Linking the execution file
#
$(TARGET): $(CU_OBJS) $(CPP_OBJS)
	$(CC) -o $@ $(CU_OBJS) $(CPP_OBJS) $(LDFLAGS)

#
# Generate dependencies
#
-include $(CPP_DEPS) $(CU_DEPS)

#
# Create directories if they don't exist
#
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

#
# Cleaning the files
#
clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d $(TARGET) *~
	rm -rf $(BIN_DIR) $(OBJ_DIR)
