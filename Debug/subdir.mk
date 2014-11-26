################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Pcon.c \
../char_fsm.c \
../cmd_fsm.c \
../serial_io.c \
../trace.c 

OBJS += \
./Pcon.o \
./char_fsm.o \
./cmd_fsm.o \
./serial_io.o \
./trace.o 

C_DEPS += \
./Pcon.d \
./char_fsm.d \
./cmd_fsm.d \
./serial_io.d \
./trace.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I/usr/arm-linux-gnueabihf/include/c++ -I/usr/arm-linux-gnueabihf/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


