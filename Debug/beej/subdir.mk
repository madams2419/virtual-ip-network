################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../beej/listener.c \
../beej/talker.c 

OBJS += \
./beej/listener.o \
./beej/talker.o 

C_DEPS += \
./beej/listener.d \
./beej/talker.d 


# Each subdirectory must supply rules for building sources it contributes
beej/%.o: ../beej/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


