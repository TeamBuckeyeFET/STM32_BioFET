################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/main.c \
../Core/Src/mcp23s17.c \
../Core/Src/w25q32.c 

C_DEPS += \
./Core/Src/main.d \
./Core/Src/mcp23s17.d \
./Core/Src/w25q32.d 

OBJS += \
./Core/Src/main.o \
./Core/Src/mcp23s17.o \
./Core/Src/w25q32.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	$(error unable to generate command line)

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/mcp23s17.cyclo ./Core/Src/mcp23s17.d ./Core/Src/mcp23s17.o ./Core/Src/mcp23s17.su ./Core/Src/w25q32.cyclo ./Core/Src/w25q32.d ./Core/Src/w25q32.o ./Core/Src/w25q32.su

.PHONY: clean-Core-2f-Src

