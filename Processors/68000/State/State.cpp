//
//  State.cpp
//  Clock Signal
//
//  Created by Thomas Harte on 14/05/2020.
//  Copyright © 2020 Thomas Harte. All rights reserved.
//

#include "State.hpp"

using namespace CPU::MC68000;

State::State(const ProcessorBase &src): State() {
	for(int c = 0; c < 7; ++c) {
		registers.address[c] = src.address_[c].full;
		registers.data[c] = src.data_[c].full;
	}
	registers.data[7] = src.data_[7].full;
	registers.user_stack_pointer = src.is_supervisor_ ? src.stack_pointers_[0].full : src.address_[7].full;
	registers.supervisor_stack_pointer = src.is_supervisor_ ? src.address_[7].full : src.stack_pointers_[1].full;
	registers.status = src.get_status();
	registers.program_counter = src.program_counter_.full;
	registers.prefetch = src.prefetch_queue_.full;
}

void State::apply(ProcessorBase &target) {
}

// Boilerplate follows here, to establish 'reflection'.
State::State() {
	if(needs_declare()) {
		DeclareField(registers);
		DeclareField(execution_state);
		DeclareField(inputs);
	}
}

State::Registers::Registers() {
	if(needs_declare()) {
		DeclareField(data);
		DeclareField(address);
		DeclareField(user_stack_pointer);
		DeclareField(supervisor_stack_pointer);
		DeclareField(status);
		DeclareField(program_counter);
		DeclareField(prefetch);
	}
}

State::ExecutionState::ExecutionState() {
	if(needs_declare()) {
	}
}

State::Inputs::Inputs() {
	if(needs_declare()) {
	}
}
