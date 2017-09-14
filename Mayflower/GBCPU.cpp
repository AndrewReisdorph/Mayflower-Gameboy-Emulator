#include "GBCPU.h"
#include "OpCodes.h"

using namespace std;

#define LOGGING 0

GBCPU::GBCPU(GBMMU *MMU, EmulatorEngine *Emulator)
{
	m_MMU = MMU;
	m_Emulator = Emulator;

	m_OpHandlers[OP_MISC_CONTROL] = &GBCPU::ExecuteMiscControlOp;
	m_OpHandlers[OP_JUMP_CALL] = &GBCPU::ExecuteJumpCallOp;
	m_OpHandlers[OP_LD_STR_MV_8] = &GBCPU::ExecuteLdStrMv8Op;
	m_OpHandlers[OP_LD_STR_MV_16] = &GBCPU::ExecuteLdStrMv16Op;
	m_OpHandlers[OP_ALU_8] = &GBCPU::Execute8BitAluOp;
	m_OpHandlers[OP_ALU_16] = &GBCPU::Execute16BitAluOp;
	m_OpHandlers[OP_ROT_SHFT_BIT_8] = &GBCPU::ExecuteRotShftBitOp;
	m_OpHandlers[OP_CB] = &GBCPU::ExecuteCBPrefixOp;
#if LOGGING
	m_logfile.open("C:\\Users\\Andrew\\Downloads\\gameboy stuff\\03-op sp,hl.lg");
#endif
}

GBCPU::~GBCPU()
{
	m_logfile.close();
}

Instruction &GBCPU::GetInstruction(opcode OpCode)
{
	return InstructionSet[OpCode];
}

Instruction &GBCPU::GetCBInstruction(opcode OpCode)
{
	return CBPrefixInstructions[OpCode];
}

int GBCPU::ExecuteNextOp()
{
	int pcofop = m_ProgramCounter;
	opcode NextOpCode = m_MMU->ReadMemory8(m_ProgramCounter);
	Instruction &NextOp = InstructionSet[NextOpCode];
	bool cb_op = false;
	int NumCycles = 0;
	m_ProgramCounter++;

	if (NextOpCode == 0xCB)
	{
		cb_op = true;
		NextOpCode = m_MMU->ReadMemory8(m_ProgramCounter);
		NextOp = CBPrefixInstructions[NextOpCode];
		m_ProgramCounter++;
	}
	else if (NextOpCode == 0x10) // STOP
	{
		// STOP instruction has a length of two bytes
		m_ProgramCounter++;
	}

	if (NextOp.OpArgType == ARG_MISSING)
	{
		cout << "Arg not found: " << int(NextOpCode) << endl;
		throw std::exception();
	}

	switch (NextOp.OpArgType)
	{
	case ARG_UNSIGNED_8:
		NextOp.Argument.uint8 = (char)m_MMU->ReadMemory8(m_ProgramCounter);
		m_ProgramCounter++;
		break;
	case ARG_SIGNED_8:
		NextOp.Argument.sint8 = (signed char)m_MMU->ReadMemory8(m_ProgramCounter);
		m_ProgramCounter++;
		break;
	case ARG_UNSIGNED_16:
		NextOp.Argument.uint16 = (unsigned short)(m_MMU->ReadMemory8(m_ProgramCounter+1) << 8) + m_MMU->ReadMemory8(m_ProgramCounter);
		m_ProgramCounter += 2;
		break;
	}

#if LOGGING
	if (pcofop >= 0x100)
	{
		m_logfile << hex << pcofop << ": A:" << (int)m_RegisterA << " B:" << (int)m_RegisterB << " C:" << (int)m_RegisterC << " D:" << (int)m_RegisterD << " E:" << (int)m_RegisterE << " F:" << (int)m_RegisterF.all << " H:" << (int)m_RegisterH << " L:" << (int)m_RegisterL << " SP:" << (int)m_StackPointer  << "\n";
	}
#endif


	if (cb_op)
	{
		NumCycles = ExecuteCBPrefixOp(NextOp);
	}
	else
	{
		if (NextOp.Type >= 7)
		{
			cout << "sheeit" << endl;
		}

		NumCycles = invoke(m_OpHandlers[NextOp.Type], this, NextOp);

	}

	m_ReferenceProgramCounter = m_ProgramCounter;

	return NumCycles;
}

word GBCPU::GetProgramCounter()
{
	return m_ProgramCounter;
}

word GBCPU::GetReferenceProgramCounter()
{
	return m_ReferenceProgramCounter;
}

void GBCPU::SetProgramCounter(word PCValue)
{
	m_ProgramCounter = PCValue;
}

word GBCPU::GetRegisterValue(RegisterID RegisterID)
{
	word RegisterValue;

	switch (RegisterID)
	{
	case REGISTER_AF:
		RegisterValue = M_RegisterAF;
		break;
	case REGISTER_BC:
		RegisterValue = M_RegisterBC;
		break;
	case REGISTER_DE:
		RegisterValue = M_RegisterDE;
		break;
	case REGISTER_HL:
		RegisterValue = M_RegisterHL;
		break;
	case REGISTER_SP:
		RegisterValue = m_StackPointer;
		break;
	case REGISTER_PC:
		RegisterValue = m_ProgramCounter;
		break;
	case REGISTER_IME:
		RegisterValue = m_RegisterIME;
		break;
	case REGISTER_IMA:
		RegisterValue = m_RegisterIMA;
		break;
	}

	return RegisterValue;
}

void GBCPU::HandleInterrupts()
{
	bool Halted = m_Emulator->GetHalted();

	// Interrupt Enable/Disabled must be done after
	// the following DI/EI instruction
	if (m_PendingInterruptStatusUpdate)
	{
		if (m_InterruptInstructionJustExecuted)
		{
			m_InterruptInstructionJustExecuted = false;
		}
		else
		{
			m_RegisterIME = m_ReferenceInterruptStatus;
			m_PendingInterruptStatusUpdate = false;
		}
	}

	static const interrupt Interrupts[NUM_INTERRUPTS] = {
		{ VBLANK_INTERRUPT,   VBLANK_INT_ADDR },
		{ LCD_STAT_INTERRUPT, LCD_STAT_INT_ADDR },
		{ TIMER_INTERRUPT,    TIMER_INT_ADDR },
		{ SERIAL_INTERRUPT,   SERIAL_INT_ADDR },
		{ JOYPAD_INTERRUPT,   JOYPAD_INT_ADDR } };

	if (m_RegisterIME || Halted)
	{
		byte InterruptsEnabled = m_MMU->ReadMemory8(IE_REGISTER);
		byte InterruptsRequested = m_MMU->ReadMemory8(IF_REGISTER);

		for (int InterruptIter = 0; InterruptIter < NUM_INTERRUPTS; InterruptIter++)
		{
			if (M_ServiceInterrupt(InterruptsRequested, InterruptsEnabled, Interrupts[InterruptIter].Flag))
			{
				if (Halted)
				{
					m_Emulator->ClearHalted();
				}
				
				if(m_RegisterIME)
				{
					M_ClearBit(InterruptsRequested, Interrupts[InterruptIter].Flag);
					m_MMU->WriteMemory8(IF_REGISTER, InterruptsRequested);

					m_RegisterIME = 0;
					m_MMU->StackPush(m_ProgramCounter);
					m_ProgramCounter = Interrupts[InterruptIter].Address;
					m_ReferenceProgramCounter = m_ProgramCounter;

				}

			}
		}
	}

}


int GBCPU::ExecuteJumpCallOp(Instruction &Op)
{
	int cycles_used = 0;

	switch (Op.OpCode)
	{
	case 0x18: // JR r8
		m_ProgramCounter += Op.Argument.sint8;
		cycles_used = Op.NumCycles[0];
		break;
	case 0x20: // JR NZ,r8
		if (m_RegisterF.Z)
		{
			cycles_used = Op.NumCycles[0];
		}
		else
		{
			m_ProgramCounter += Op.Argument.sint8;
			cycles_used = Op.NumCycles[1];
		}
		break;
	case 0x28: // JR Z,r8
		if (m_RegisterF.Z)
		{
			m_ProgramCounter += Op.Argument.sint8;
			cycles_used = Op.NumCycles[1];
		}
		else
		{
			cycles_used = Op.NumCycles[0];
		}
		break;
	case 0x30: // JR NC,r8
		if (m_RegisterF.C)
		{
			cycles_used = Op.NumCycles[0];
		}
		else
		{
			m_ProgramCounter += Op.Argument.sint8;
			cycles_used = Op.NumCycles[1];
		}
		break;
	case 0x38: // JR C,r8
		if (m_RegisterF.C)
		{
			m_ProgramCounter += Op.Argument.sint8;
			cycles_used = Op.NumCycles[1];
		}
		else
		{
			cycles_used = Op.NumCycles[0];
		}
		break;
	case 0xC0: // RET NZ
		if (m_RegisterF.Z)
		{
			cycles_used = Op.NumCycles[0];
		}
		else
		{
			m_ProgramCounter = m_MMU->StackPop();
			cycles_used = Op.NumCycles[1];
		}
		break;
	case 0xC2: // JP NZ,a16
		if (m_RegisterF.Z)
		{
			cycles_used = Op.NumCycles[0];
		}
		else
		{
			m_ProgramCounter = Op.Argument.uint16;
			cycles_used = Op.NumCycles[1];
		}
		break;
	case 0xC3: // JP a16
		m_ProgramCounter = Op.Argument.uint16;
		cycles_used = Op.NumCycles[0];
		break;
	case 0xC4: // CALL NZ,a16
		if (m_RegisterF.Z)
		{
			cycles_used = Op.NumCycles[0];
		}
		else
		{
			m_MMU->StackPush(m_ProgramCounter);
			m_ProgramCounter = Op.Argument.uint16;
			cycles_used = Op.NumCycles[1];
		}
		break;
	case 0xC7: // RST 00H
		m_MMU->StackPush(m_ProgramCounter);
		m_ProgramCounter = 0x00;
		break;
	case 0xC8: // RET Z
		if (m_RegisterF.Z)
		{
			m_ProgramCounter = m_MMU->StackPop();
			cycles_used = Op.NumCycles[1];
		}
		else
		{
			cycles_used = Op.NumCycles[0];
		}
		break;
	case 0xC9: // RET
		m_ProgramCounter = m_MMU->StackPop();
		cycles_used = Op.NumCycles[0];
		break;
	case 0xCA: // JP Z,a16
		if (m_RegisterF.Z)
		{
			m_ProgramCounter = Op.Argument.uint16;
			cycles_used = Op.NumCycles[1];
		}
		else
		{
			cycles_used = Op.NumCycles[0];
		}
		break;
	case 0xCC: // CALL Z,a16
		if (m_RegisterF.Z)
		{
			m_MMU->StackPush(m_ProgramCounter);
			m_ProgramCounter = Op.Argument.uint16;
			cycles_used = Op.NumCycles[1];
		}
		else
		{
			cycles_used = Op.NumCycles[0];
		}
		break;
	case 0xCD: // CALL a16
		m_MMU->StackPush(m_ProgramCounter);
		m_ProgramCounter = Op.Argument.uint16;
		cycles_used = Op.NumCycles[0];
		break;
	case 0xCF: // RST 08H
		m_MMU->StackPush(m_ProgramCounter);
		m_ProgramCounter = 0x08;
		break;
	case 0xD0: // RET NC
		if (m_RegisterF.C)
		{
			cycles_used = Op.NumCycles[0];
		}
		else
		{
			m_ProgramCounter = m_MMU->StackPop();
			cycles_used = Op.NumCycles[1];
		}
		break;
	case 0xD2: // JP NC,a16
		if (m_RegisterF.C)
		{
			cycles_used = Op.NumCycles[0];
		}
		else
		{
			m_ProgramCounter = Op.Argument.uint16;
			cycles_used = Op.NumCycles[1];
		}
		break;
	case 0xD4: // CALL NC,a16
		if (m_RegisterF.C)
		{
			cycles_used = Op.NumCycles[0];
		}
		else
		{
			m_MMU->StackPush(m_ProgramCounter);
			m_ProgramCounter = Op.Argument.uint16;
			cycles_used = Op.NumCycles[1];
		}
		break;
	case 0xD7: // RST 10H
		m_MMU->StackPush(m_ProgramCounter);
		m_ProgramCounter = 0x10;
		break;
	case 0xD8: // RET C
		if (m_RegisterF.C)
		{
			m_ProgramCounter = m_MMU->StackPop();
			cycles_used = Op.NumCycles[1];
		}
		else
		{
			cycles_used = Op.NumCycles[0];
		}
		break;
	case 0xD9: // RETI
		m_ProgramCounter = m_MMU->StackPop();
		cycles_used = Op.NumCycles[0];
		m_RegisterIME = 1;
		break;
	case 0xDA: // JP C,a16
		if (m_RegisterF.C)
		{
			m_ProgramCounter = Op.Argument.uint16;
			cycles_used = Op.NumCycles[1];
		}
		else
		{
			cycles_used = Op.NumCycles[0];
		}
		break;
	case 0xDC: // CALL C,a16
		if (m_RegisterF.C)
		{
			m_MMU->StackPush(m_ProgramCounter);
			m_ProgramCounter = Op.Argument.uint16;
			cycles_used = Op.NumCycles[1];
		}
		else
		{
			cycles_used = Op.NumCycles[0];
		}
		break;
	case 0xDF: // RST 18H
		m_MMU->StackPush(m_ProgramCounter);
		m_ProgramCounter = 0x18;
		break;
	case 0xE7: // RST 20H
		m_MMU->StackPush(m_ProgramCounter);
		m_ProgramCounter = 0x20;
		break;
	case 0xE9: // JP (HL)
		m_ProgramCounter = M_RegisterHL;
		cycles_used = Op.NumCycles[0];
		break;
	case 0xEF: // RST 28H
		m_MMU->StackPush(m_ProgramCounter);
		m_ProgramCounter = 0x28;
		break;
	case 0xF7: // RST 30H
		m_MMU->StackPush(m_ProgramCounter);
		m_ProgramCounter = 0x30;
		break;
	case 0xFF: // RST 38H
		m_MMU->StackPush(m_ProgramCounter);
		m_ProgramCounter = 0x38;
		break;
	default:
		cout << "Instruction not defined: " << int(Op.OpCode) << endl;
		throw std::exception();
	}

	return cycles_used;
}

int GBCPU::ExecuteMiscControlOp(Instruction &Op)
{
	int cycles_used = 0;
	bool not_implemented = false;

	switch (Op.OpCode)
	{
	case 0x00: // NOP
		break;
	case 0x10: // STOP
		m_Emulator->SetStopped();
		break;
	case 0x76: // HALT
		// TODO=MAKE THIS WORK
		m_Emulator->SetHalted();
		break;
	case 0xF3: // DI
		m_RegisterIME = 0;
		break;
	case 0xFB: // EI
		M_EnableInterrupts();
		break;
	default:
		cout << "Instruction not defined: " << int(Op.OpCode) << endl;
		throw std::exception();
	}

	if (not_implemented)
	{
		cout << "Instruction not implemented: " << int(Op.OpCode) << endl;
		cout << "PC: " << hex << m_ReferenceProgramCounter << endl;
		throw std::exception();
	}

	cycles_used = Op.NumCycles[0];

	return cycles_used;
}

int GBCPU::Execute8BitAluOp(Instruction &Op)
{
	int cycles_used = 0;

	if (Op.OpCode <= 0x99)
	{
		switch (Op.OpCode)
		{
		case 0x04: // INC B
			M_INC_Reg(m_RegisterB);
			break;
		case 0x05: // DEC B
			M_DEC_Reg(m_RegisterB);
			break;
		case 0x0C: // INC C
			M_INC_Reg(m_RegisterC);
			break;
		case 0x0D: // DEC C
			M_DEC_Reg(m_RegisterC);
			break;
		case 0x14: // INC D
			M_INC_Reg(m_RegisterD);
			break;
		case 0x15: // DEC D
			M_DEC_Reg(m_RegisterD);
			break;
		case 0x1C: // INC E
			M_INC_Reg(m_RegisterE);
			break;
		case 0x1D: // DEC E
			M_DEC_Reg(m_RegisterE);
			break;
		case 0x24: // INC H
			M_INC_Reg(m_RegisterH);
			break;
		case 0x25: // DEC H
			M_DEC_Reg(m_RegisterH);
			break;
		case 0x27: // DAA
		{
			byte Offset = 0;
			if (m_RegisterF.H || (!m_RegisterF.N && (m_RegisterA & 0x0F) > 9))
			{
				Offset += 0x06;
			}
			if (m_RegisterF.C || (!m_RegisterF.N && (m_RegisterA > 0x99)))
			{
				Offset += 0x60;
				m_RegisterF.C = 1;
			}

			m_RegisterA += m_RegisterF.N ? -Offset : Offset;

			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.H = 0;
		}
		break;
		case 0x2C: // INC L
			M_INC_Reg(m_RegisterL);
			break;
		case 0x2D: // DEC L
			M_DEC_Reg(m_RegisterL);
			break;
		case 0x2F: // CPL
			m_RegisterA = ~(m_RegisterA);
			m_RegisterF.H = 1;
			m_RegisterF.N = 1;
			break;
		case 0x34: // INC (HL)
		{
			byte ValAtHL = m_MMU->ReadMemory8(M_RegisterHL);
			m_MMU->WriteMemory8(M_RegisterHL, ValAtHL + 1);
			m_RegisterF.Z = (m_MMU->ReadMemory8(M_RegisterHL) == 0);
			m_RegisterF.N = 0;
			m_RegisterF.H = (((ValAtHL & 0xF) + 1) > 0xF);
		}
		break;
		case 0x35: // DEC (HL)
		{
			byte ValAtHL = m_MMU->ReadMemory8(M_RegisterHL);
			m_MMU->WriteMemory8(M_RegisterHL, ValAtHL - 1);
			m_RegisterF.Z = (m_MMU->ReadMemory8(M_RegisterHL) == 0);
			m_RegisterF.N = 1;
			m_RegisterF.H = ((ValAtHL & 0x0F) == 0);
		}
		break;
		case 0x37: // SCF
			m_RegisterF.C = 1;
			m_RegisterF.N = 0;
			m_RegisterF.H = 0;
			break;
		case 0x3C: // INC A
			M_INC_Reg(m_RegisterA);
			break;
		case 0x3D: // DEC A
			M_DEC_Reg(m_RegisterA);
			break;
		case 0x3F: // CCF
			m_RegisterF.C = !m_RegisterF.C;
			m_RegisterF.H = 0;
			m_RegisterF.N = 0;
			break;
		case 0x80: // ADD A,B
			m_RegisterF.C = (((int)(m_RegisterA)+(int)(m_RegisterB)) > 0xFF);
			m_RegisterF.H = (((m_RegisterA & 0x0F) + (m_RegisterB & 0x0F)) > 0x0F);
			m_RegisterA += m_RegisterB;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			break;			break;
		case 0x81: // ADD A,C
			m_RegisterF.C = (((int)(m_RegisterA)+(int)(m_RegisterC)) > 0xFF);
			m_RegisterF.H = (((m_RegisterA & 0x0F) + (m_RegisterC & 0x0F)) > 0x0F);
			m_RegisterA += m_RegisterC;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			break;
		case 0x82: // ADD A,D
			m_RegisterF.C = (((int)(m_RegisterA)+(int)(m_RegisterD)) > 0xFF);
			m_RegisterF.H = (((m_RegisterA & 0x0F) + (m_RegisterD & 0x0F)) > 0x0F);
			m_RegisterA += m_RegisterD;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			break;
		case 0x83: // ADD A,E
			m_RegisterF.C = (((int)(m_RegisterA)+(int)(m_RegisterE)) > 0xFF);
			m_RegisterF.H = (((m_RegisterA & 0x0F) + (m_RegisterE & 0x0F)) > 0x0F);
			m_RegisterA += m_RegisterE;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			break;
		case 0x84: // ADD A,H
			m_RegisterF.C = (((int)(m_RegisterA)+(int)(m_RegisterH)) > 0xFF);
			m_RegisterF.H = (((m_RegisterA & 0x0F) + (m_RegisterH & 0x0F)) > 0x0F);
			m_RegisterA += m_RegisterH;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			break;
		case 0x85: // ADD A,L
			m_RegisterF.C = (((int)(m_RegisterA)+(int)(m_RegisterL)) > 0xFF);
			m_RegisterF.H = (((m_RegisterA & 0x0F) + (m_RegisterL & 0x0F)) > 0x0F);
			m_RegisterA += m_RegisterL;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			break;
		case 0x86: // ADD A,(HL)
		{
			byte ValueAtHL = m_MMU->ReadMemory8(M_RegisterHL);
			m_RegisterF.C = ((m_RegisterA + ValueAtHL) > 0xFF);
			m_RegisterF.H = (((m_RegisterA & 0x0F) + (ValueAtHL & 0x0F)) > 0x0F);
			m_RegisterA += ValueAtHL;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
		}
			break;
		case 0x87: // ADD A,A
			m_RegisterF.C = (((int)(m_RegisterA)+(int)(m_RegisterA)) > 0xFF);
			m_RegisterF.H = (((m_RegisterA & 0x0F) + (m_RegisterA & 0x0F)) > 0x0F);
			m_RegisterA += m_RegisterA;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			break;
		case 0x88: // ADC A,B
			M_CB_ADC(m_RegisterB);
			break;
		case 0x89: // ADC A,C
			M_CB_ADC(m_RegisterC);
			break;
		case 0x8A: // ADC A,D
			M_CB_ADC(m_RegisterD);
			break;
		case 0x8B: // ADC A,E
			M_CB_ADC(m_RegisterE);
			break;
		case 0x8C: // ADC A,H
			M_CB_ADC(m_RegisterH);
			break;
		case 0x8D: // ADC A,L
			M_CB_ADC(m_RegisterL);
			break;
		case 0x8E: // ADC A,(HL)
			M_CB_ADC(m_MMU->ReadMemory8(M_RegisterHL));
			break;
		case 0x8F: // ADC A,A
			M_CB_ADC(m_RegisterA);
			break;
		case 0x90: // SUB B
			m_RegisterF.H = ((int)(m_RegisterA & 0xF) - (int)(m_RegisterB & 0xF)) < 0;
			m_RegisterF.C = m_RegisterA < m_RegisterB;
			m_RegisterF.N = 1;
			m_RegisterA -= m_RegisterB;
			m_RegisterF.Z = (m_RegisterA == 0);
			break;
		case 0x91: // SUB C
			m_RegisterF.H = ((int)(m_RegisterA & 0xF) - (int)(m_RegisterC & 0xF)) < 0;
			m_RegisterF.C = m_RegisterA < m_RegisterC;
			m_RegisterF.N = 1;
			m_RegisterA -= m_RegisterC;
			m_RegisterF.Z = (m_RegisterA == 0);
			break;
		case 0x92: // SUB D
			m_RegisterF.H = ((int)(m_RegisterA & 0xF) - (int)(m_RegisterD & 0xF)) < 0;
			m_RegisterF.C = m_RegisterA < m_RegisterD;
			m_RegisterF.N = 1;
			m_RegisterA -= m_RegisterD;
			m_RegisterF.Z = (m_RegisterA == 0);
			break;
		case 0x93: // SUB E
			m_RegisterF.H = ((int)(m_RegisterA & 0xF) - (int)(m_RegisterE & 0xF)) < 0;
			m_RegisterF.C = m_RegisterA < m_RegisterE;
			m_RegisterF.N = 1;
			m_RegisterA -= m_RegisterE;
			m_RegisterF.Z = (m_RegisterA == 0);
			break;
		case 0x94: // SUB H
			m_RegisterF.H = ((int)(m_RegisterA & 0xF) - (int)(m_RegisterH & 0xF)) < 0;
			m_RegisterF.C = m_RegisterA < m_RegisterH;
			m_RegisterF.N = 1;
			m_RegisterA -= m_RegisterH;
			m_RegisterF.Z = (m_RegisterA == 0);
			break;
		case 0x95: // SUB L
			m_RegisterF.H = ((int)(m_RegisterA & 0xF) - (int)(m_RegisterL & 0xF)) < 0;
			m_RegisterF.C = m_RegisterA < m_RegisterL;
			m_RegisterF.N = 1;
			m_RegisterA -= m_RegisterL;
			m_RegisterF.Z = (m_RegisterA == 0);
			break;
		case 0x96: // SUB (HL)
		{
			byte HLval = m_MMU->ReadMemory8(M_RegisterHL);
			m_RegisterF.H = ((m_RegisterA & 0xF) - (HLval & 0xF)) < 0;
			m_RegisterF.C = m_RegisterA < HLval;
			m_RegisterF.N = 1;
			m_RegisterA -= HLval;
			m_RegisterF.Z = (m_RegisterA == 0);
		}
			break;
		case 0x97: // SUB A
			m_RegisterA = 0;
			m_RegisterF.Z = 1;
			m_RegisterF.N = 1;
			m_RegisterF.H = 0;
			m_RegisterF.C = 0;
			break;
		case 0x98: // SBC A,B
			M_SBC_Reg(m_RegisterB);
			break;
		case 0x99: // SBC A,C
			M_SBC_Reg(m_RegisterC);
			break;
		default:
			cout << "Instruction not defined: " << int(Op.OpCode) << endl;
			throw std::exception();
		}
	}
	else
	{
		switch (Op.OpCode)
		{
		case 0x9A: // SBC A,D
			M_SBC_Reg(m_RegisterD);
			break;
		case 0x9B: // SBC A,E
			M_SBC_Reg(m_RegisterE);
			break;
		case 0x9C: // SBC A,H
			M_SBC_Reg(m_RegisterH);
			break;
		case 0x9D: // SBC A,L
			M_SBC_Reg(m_RegisterL);
			break;
		case 0x9E: // SBC A,(HL)
			M_SBC_Reg(m_MMU->ReadMemory8(M_RegisterHL));
			break;
		case 0x9F: // SBC A,A
			M_SBC_Reg(m_RegisterA);
			break;
		case 0xA0: // AND B
			M_AND_Reg(m_RegisterB);
			break;
		case 0xA1: // AND C
			M_AND_Reg(m_RegisterC);
			break;
		case 0xA2: // AND D
			M_AND_Reg(m_RegisterD);
			break;
		case 0xA3: // AND E
			M_AND_Reg(m_RegisterE);
			break;
		case 0xA4: // AND H
			M_AND_Reg(m_RegisterH);
			break;
		case 0xA5: // AND L
			M_AND_Reg(m_RegisterL);
			break;
		case 0xA6: // AND (HL)
			M_AND_Reg(m_MMU->ReadMemory8(M_RegisterHL));
			break;
		case 0xA7: // AND A
			M_AND_Reg(m_RegisterA);
			break;
		case 0xA8: // XOR B
			M_XOR_Reg(m_RegisterB);
			break;
		case 0xA9: // XOR C
			M_XOR_Reg(m_RegisterC);
			break;
		case 0xAA: // XOR D
			M_XOR_Reg(m_RegisterD);
			break;
		case 0xAB: // XOR E
			M_XOR_Reg(m_RegisterE);
			break;
		case 0xAC: // XOR H
			M_XOR_Reg(m_RegisterH);
			break;
		case 0xAD: // XOR L
			M_XOR_Reg(m_RegisterL);
			break;
		case 0xAE: // XOR (HL)
			M_XOR_Reg(m_MMU->ReadMemory8(M_RegisterHL));
			break;
		case 0xAF: // XOR A
			m_RegisterA = 0;
			m_RegisterF.H = 0;
			m_RegisterF.C = 0;
			m_RegisterF.N = 0;
			m_RegisterF.Z = 1;
			break;
		case 0xB0: // OR B
			M_OR_Reg(m_RegisterB);
			break;
		case 0xB1: // OR C
			M_OR_Reg(m_RegisterC);
			break;
		case 0xB2: // OR D
			M_OR_Reg(m_RegisterD);
			break;
		case 0xB3: // OR E
			M_OR_Reg(m_RegisterE);
			break;
		case 0xB4: // OR H
			M_OR_Reg(m_RegisterH);
			break;
		case 0xB5: // OR L
			M_OR_Reg(m_RegisterL);
			break;
		case 0xB6: // OR (HL)
			M_OR_Reg(m_MMU->ReadMemory8(M_RegisterHL));
			break;
		case 0xB7: // OR A
			M_OR_Reg(m_RegisterA);
			break;
		case 0xB8: // CP B
			M_CPReg(m_RegisterB);
			break;
		case 0xB9: // CP C
			M_CPReg(m_RegisterC);
			break;
		case 0xBA: // CP D
			M_CPReg(m_RegisterD);
			break;
		case 0xBB: // CP E
			M_CPReg(m_RegisterE);
			break;
		case 0xBC: // CP H
			M_CPReg(m_RegisterH);
			break;
		case 0xBD: // CP L
			M_CPReg(m_RegisterL);
			break;
		case 0xBE: // CP (HL)
			M_CPReg(m_MMU->ReadMemory8(M_RegisterHL));
			break;
		case 0xBF: // CP A
			M_CPReg(m_RegisterA);
			break;
		case 0xC6: // ADD A,d8
			m_RegisterF.C = M_CheckBit7Carry(m_RegisterA, Op.Argument.uint8);
			m_RegisterF.H = M_CheckBit3Carry(m_RegisterA, Op.Argument.uint8);
			m_RegisterA += Op.Argument.uint8;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			break;
		case 0xCE: // ADC A,d8
		{
			byte OriginalC = m_RegisterF.C;
			m_RegisterF.C = M_CheckBit7Carry(m_RegisterA, (Op.Argument.uint8 + OriginalC));
			m_RegisterF.H = ((((m_RegisterA) & 0x0F) + ((Op.Argument.uint8) & 0x0F) + OriginalC) > 0x0F);
			m_RegisterA += (Op.Argument.uint8 + OriginalC) & 0xFF;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
		}
		break;
		case 0xD6: // SUB d8
			m_RegisterF.H = ((m_RegisterA & 0xF) - (Op.Argument.uint8 & 0xF)) < 0;
			m_RegisterF.C = m_RegisterA < Op.Argument.uint8;
			m_RegisterF.N = 1;
			m_RegisterA -= Op.Argument.uint8;
			m_RegisterF.Z = (m_RegisterA == 0);
			break;
		case 0xDE: // SBC A,d8
		{
			byte tempCarryFlag = m_RegisterF.C;
			m_RegisterF.H = ((m_RegisterA & 0xF) - ((Op.Argument.uint8) & 0xF) - tempCarryFlag) < 0;
			m_RegisterF.C = m_RegisterA < (Op.Argument.uint8 + tempCarryFlag);
			m_RegisterF.N = 1;
			m_RegisterA -= (Op.Argument.uint8 + tempCarryFlag);
			m_RegisterF.Z = (m_RegisterA == 0);
		}
			break;
		case 0xE6: // AND d8
			m_RegisterA &= Op.Argument.uint8;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			m_RegisterF.H = 1;
			m_RegisterF.C = 0;
			break;
		case 0xEE: // XOR d8
			m_RegisterA ^= Op.Argument.uint8;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			m_RegisterF.H = 0;
			m_RegisterF.C = 0;
			break;
		case 0xF6: // OR d8
			m_RegisterA |= Op.Argument.uint8;
			m_RegisterF.Z = (m_RegisterA == 0);
			m_RegisterF.N = 0;
			m_RegisterF.H = 0;
			m_RegisterF.C = 0;
			break;
		case 0xFE: // CP d8
			m_RegisterF.Z = (m_RegisterA == Op.Argument.uint8);
			m_RegisterF.N = 1;
			m_RegisterF.H = ((m_RegisterA & 0x0F) < (Op.Argument.uint8 & 0x0F));
			m_RegisterF.C = (m_RegisterA < Op.Argument.uint8);
			break;
		default:
			cout << "Instruction not defined: " << int(Op.OpCode) << endl;
			throw std::exception();
		}
	}

	cycles_used = Op.NumCycles[0];

	return cycles_used;
}

int GBCPU::ExecuteLdStrMv16Op(Instruction &Op)
{
	int cycles_used = 0;

	switch (Op.OpCode)
	{
	case 0x01: // LD BC,d16
		m_RegisterB = (Op.Argument.uint16 & 0xFF00) >> 8;
		m_RegisterC = (Op.Argument.uint16 & 0x00FF);
		break;
	case 0x08: // LD (a16),SP
		m_MMU->WriteMemory16(Op.Argument.uint16, m_StackPointer);
		break;
	case 0x11: // LD DE,d16
		m_RegisterD = (Op.Argument.uint16 & 0xFF00) >> 8;
		m_RegisterE = (Op.Argument.uint16 & 0x00FF);
		break;
	case 0x21: // LD HL,d16
		m_RegisterH = (Op.Argument.uint16 & 0xFF00) >> 8;
		m_RegisterL = (Op.Argument.uint16 & 0x00FF);
		break;
	case 0x31: // LD SP,d16
		m_StackPointer = Op.Argument.uint16;
		break;
	case 0xC1: // POP BC
	{
		word StackValue = m_MMU->StackPop();
		m_RegisterB = (StackValue & 0xFF00) >> 8;
		m_RegisterC = (StackValue & 0x00FF);
		break;
	}
	case 0xC5: // PUSH BC
		m_MMU->StackPush(M_RegisterBC);
		break;
	case 0xD1: // POP DE
	{
		word StackWord = m_MMU->StackPop();
		m_RegisterD = (StackWord & 0xFF00) >> 8;
		m_RegisterE = StackWord & 0x00FF;
		break;
	}
	case 0xD5: // PUSH DE
	{
		m_MMU->StackPush(M_RegisterDE);
		break;
	}
	case 0xE1: // POP HL
	{
		word StackWord = m_MMU->StackPop();
		m_RegisterH = (StackWord & 0xFF00) >> 8;
		m_RegisterL = StackWord & 0x00FF;
		break;
	}
	case 0xE5: // PUSH HL
	{
		m_MMU->StackPush(M_RegisterHL);
		break;
	}
	case 0xF1: // POP AF
	{
		word StackWord = m_MMU->StackPop();
		m_RegisterA = (StackWord & 0xFF00) >> 8;
		// You can only write to the top 4 bits of the F register
		m_RegisterF.all = StackWord & 0x00F0;
		break;
	}
	case 0xF5: // PUSH AF
	{
		m_MMU->StackPush(M_RegisterAF);
		break;
	}
	case 0xF8: // LD HL,SP+r8
	{
		word temp = m_StackPointer + Op.Argument.sint8;
		M_SetHL(temp);
		m_RegisterF.Z = 0;
		m_RegisterF.N = 0;
		m_RegisterF.H = ((m_StackPointer & 0x000F) + (Op.Argument.sint8 & 0x0F)) > 0x0F;
		m_RegisterF.C = ((m_StackPointer & 0x00FF) + (Op.Argument.sint8 & 0xFF)) > 0xFF;
	}
		break;
	case 0xF9: // LD SP,HL
		m_StackPointer = M_RegisterHL;
		break;
	default:
		cout << "Instruction not defined: " << int(Op.OpCode) << endl;
		throw std::exception();
	}

	cycles_used = Op.NumCycles[0];

	return cycles_used;
}

int GBCPU::ExecuteLdStrMv8Op(Instruction &Op)
{
	if (Op.OpCode < 0x5A)
	{
		switch (Op.OpCode)
		{
		case 0x02: // LD (BC),A
			m_MMU->WriteMemory8(M_RegisterBC, m_RegisterA);
			break;
		case 0x06: // LD B,d8
			m_RegisterB = Op.Argument.uint8;
			break;
		case 0x0A: // LD A,(BC)
			m_RegisterA = m_MMU->ReadMemory8(M_RegisterBC);
			break;
		case 0x0E: // LD C,d8
			m_RegisterC = Op.Argument.uint8;
			break;
		case 0x12: // LD (DE),A
			m_MMU->WriteMemory8(M_RegisterDE, m_RegisterA);
			break;
		case 0x16: // LD D,d8
			m_RegisterD = Op.Argument.uint8;
			break;
		case 0x1A: // LD A,(DE)
			m_RegisterA = m_MMU->ReadMemory8(M_RegisterDE);
			break;
		case 0x1E: // LD E,d8
			m_RegisterE = Op.Argument.uint8;
			break;
		case 0x22: // LD (HL+),A
			m_MMU->WriteMemory8(M_RegisterHL, m_RegisterA);
			M_IncrementHL();
			break;
		case 0x26: // LD H,d8
			m_RegisterH = Op.Argument.uint8;
			break;
		case 0x2A: // LD A,(HL+)
			m_RegisterA = m_MMU->ReadMemory8(M_RegisterHL);
			M_IncrementHL();
			break;
		case 0x2E: // LD L,d8
			m_RegisterL = Op.Argument.uint8;
			break;
		case 0x32: // LD (HL-),A
			m_MMU->WriteMemory8(M_RegisterHL, m_RegisterA);
			M_DecrementHL();
			break;
		case 0x36: // LD (HL),d8
			m_MMU->WriteMemory8(M_RegisterHL, Op.Argument.uint8);
			break;
		case 0x3A: // LD A,(HL-)
			m_RegisterA = m_MMU->ReadMemory8(M_RegisterHL);
			M_DecrementHL();
			break;
		case 0x3E: // LD A,d8
			m_RegisterA = Op.Argument.uint8;
			break;
		case 0x40: // LD B,B
			m_RegisterB = m_RegisterB;
			break;
		case 0x41: // LD B,C
			m_RegisterB = m_RegisterC;
			break;
		case 0x42: // LD B,D
			m_RegisterB = m_RegisterD;
			break;
		case 0x43: // LD B,E
			m_RegisterB = m_RegisterE;
			break;
		case 0x44: // LD B,H
			m_RegisterB = m_RegisterH;
			break;
		case 0x45: // LD B,L
			m_RegisterB = m_RegisterL;
			break;
		case 0x46: // LD B,(HL)
			m_RegisterB = m_MMU->ReadMemory8(M_RegisterHL);
			break;
		case 0x47: // LD B,A
			m_RegisterB = m_RegisterA;
			break;
		case 0x48: // LD C,B
			m_RegisterC = m_RegisterB;
			break;
		case 0x49: // LD C,C
			m_RegisterC = m_RegisterC;
			break;
		case 0x4A: // LD C,D
			m_RegisterC = m_RegisterD;
			break;
		case 0x4B: // LD C,E
			m_RegisterC = m_RegisterE;
			break;
		case 0x4C: // LD C,H
			m_RegisterC = m_RegisterH;
			break;
		case 0x4D: // LD C,L
			m_RegisterC = m_RegisterL;
			break;
		case 0x4E: // LD C,(HL)
			m_RegisterC = m_MMU->ReadMemory8(M_RegisterHL);
			break;
		case 0x4F: // LD C,A
			m_RegisterC = m_RegisterA;
			break;
		case 0x50: // LD D,B
			m_RegisterD = m_RegisterB;
			break;
		case 0x51: // LD D,C
			m_RegisterD = m_RegisterC;
			break;
		case 0x52: // LD D,D
			m_RegisterD = m_RegisterD;
			break;
		case 0x53: // LD D,E
			m_RegisterD = m_RegisterE;
			break;
		case 0x54: // LD D,H
			m_RegisterD = m_RegisterH;
			break;
		case 0x55: // LD D,L
			m_RegisterD = m_RegisterL;
			break;
		case 0x56: // LD D,(HL)
			m_RegisterD = m_MMU->ReadMemory8(M_RegisterHL);
			break;
		case 0x57: // LD D,A
			m_RegisterD = m_RegisterA;
			break;
		case 0x58: // LD E,B
			m_RegisterE = m_RegisterB;
			break;
		case 0x59: // LD E,C
			m_RegisterE = m_RegisterC;
			break;
		default:
			cout << "Instruction not defined: " << int(Op.OpCode) << endl;
			throw std::exception();
		}
	}
	else
	{
		switch (Op.OpCode)
		{
		case 0x5A: // LD E,D
			m_RegisterE = m_RegisterD;
			break;
		case 0x5B: // LD E,E
			m_RegisterE = m_RegisterE;
			break;
		case 0x5C: // LD E,H
			m_RegisterE = m_RegisterH;
			break;
		case 0x5D: // LD E,L
			m_RegisterE = m_RegisterL;
			break;
		case 0x5E: // LD E,(HL)
			m_RegisterE = m_MMU->ReadMemory8(M_RegisterHL);
			break;
		case 0x5F: // LD E,A
			m_RegisterE = m_RegisterA;
			break;
		case 0x60: // LD H,B
			m_RegisterH = m_RegisterB;
			break;
		case 0x61: // LD H,C
			m_RegisterH = m_RegisterC;
			break;
		case 0x62: // LD H,D
			m_RegisterH = m_RegisterD;
			break;
		case 0x63: // LD H,E
			m_RegisterH = m_RegisterE;
			break;
		case 0x64: // LD H,H
			m_RegisterH = m_RegisterH;
			break;
		case 0x65: // LD H,L
			m_RegisterH = m_RegisterL;
			break;
		case 0x66: // LD H,(HL)
			m_RegisterH = m_MMU->ReadMemory8(M_RegisterHL);
			break;
		case 0x67: // LD H,A
			m_RegisterH = m_RegisterA;
			break;
		case 0x68: // LD L,B
			m_RegisterL = m_RegisterB;
			break;
		case 0x69: // LD L,C
			m_RegisterL = m_RegisterC;
			break;
		case 0x6A: // LD L,D
			m_RegisterL = m_RegisterD;
			break;
		case 0x6B: // LD L,E
			m_RegisterL = m_RegisterE;
			break;
		case 0x6C: // LD L,H
			m_RegisterL = m_RegisterH;
			break;
		case 0x6D: // LD L,L
			m_RegisterL = m_RegisterL;
			break;
		case 0x6E: // LD L,(HL)
			m_RegisterL = m_MMU->ReadMemory8(M_RegisterHL);
			break;
		case 0x6F: // LD L,A
			m_RegisterL = m_RegisterA;
			break;
		case 0x70: // LD (HL),B
			m_MMU->WriteMemory8(M_RegisterHL, m_RegisterB);
			break;
		case 0x71: // LD (HL),C
			m_MMU->WriteMemory8(M_RegisterHL, m_RegisterC);
			break;
		case 0x72: // LD (HL),D
			m_MMU->WriteMemory8(M_RegisterHL, m_RegisterD);
			break;
		case 0x73: // LD (HL),E
			m_MMU->WriteMemory8(M_RegisterHL, m_RegisterE);
			break;
		case 0x74: // LD (HL),H
			m_MMU->WriteMemory8(M_RegisterHL, m_RegisterH);
			break;
		case 0x75: // LD (HL),L
			m_MMU->WriteMemory8(M_RegisterHL, m_RegisterL);
			break;
		case 0x77: // LD (HL),A
			m_MMU->WriteMemory8(M_RegisterHL, m_RegisterA);
			break;
		case 0x78: // LD A,B
			m_RegisterA = m_RegisterB;
			break;
		case 0x79: // LD A,C
			m_RegisterA = m_RegisterC;
			break;
		case 0x7A: // LD A,D
			m_RegisterA = m_RegisterD;
			break;
		case 0x7B: // LD A,E
			m_RegisterA = m_RegisterE;
			break;
		case 0x7C: // LD A,H
			m_RegisterA = m_RegisterH;
			break;
		case 0x7D: // LD A,L
			m_RegisterA = m_RegisterL;
			break;
		case 0x7E: // LD A,(HL)
			m_RegisterA = m_MMU->ReadMemory8(M_RegisterHL);
			break;
		case 0x7F: // LD A,A
			m_RegisterA = m_RegisterA;
			break;
		case 0xE0: // LDH (a8),A
			m_MMU->WriteMemory8(0xFF00 + Op.Argument.uint8, m_RegisterA);
			break;
		case 0xE2: // LD (C),A
			m_MMU->WriteMemory8(0xFF00 + m_RegisterC, m_RegisterA);
			break;
		case 0xEA: // LD (a16),A
			m_MMU->WriteMemory8(Op.Argument.uint16, m_RegisterA);
			break;
		case 0xF0: // LDH A,(a8)
			m_RegisterA = m_MMU->ReadMemory8(0xFF00 + Op.Argument.uint8);
			break;
		case 0xF2: // LD A,(C)
			m_RegisterA = m_MMU->ReadMemory8(0xFF00 + m_RegisterC);
			break;
		case 0xFA: // LD A, (a16)
			m_RegisterA = m_MMU->ReadMemory8(Op.Argument.uint16);
			break;
		default:
			cout << "Instruction not defined: " << int(Op.OpCode) << endl;
			throw std::exception();
		}
	}


	return Op.NumCycles[0];
}

int GBCPU::Execute16BitAluOp(Instruction &Op)
{
	switch (Op.OpCode)
	{
	case 0x03: // INC BC
	{
		word CurrentBCValue = M_RegisterBC;
		CurrentBCValue++;
		m_RegisterB = (CurrentBCValue & 0xFF00) >> 8;
		m_RegisterC = (CurrentBCValue & 0x00FF);
		break;
	}
	case 0x09: // ADD HL,BC
	{
		word CurrentHLValue = M_RegisterHL;
		m_RegisterF.N = 0;
		m_RegisterF.C = (M_RegisterBC + CurrentHLValue) > 0xFFFF;
		m_RegisterF.H = ((M_RegisterBC & 0x0FFF) + (CurrentHLValue & 0x0FFF) > 0x0FFF);
		CurrentHLValue += M_RegisterBC;
		M_SetHL(CurrentHLValue);
	}
	break;
	case 0x0B: // DEC BC
	{
		word CurrentBCValue = M_RegisterBC;
		CurrentBCValue--;
		m_RegisterB = (CurrentBCValue & 0xFF00) >> 8;
		m_RegisterC = (CurrentBCValue & 0x00FF);
	}
	break;
	case 0x13: // INC DE
	{
		word CurrentDEValue = M_RegisterDE;
		CurrentDEValue++;
		m_RegisterD = (CurrentDEValue & 0xFF00) >> 8;
		m_RegisterE = (CurrentDEValue & 0x00FF);
	}
	break;
	case 0x19: // ADD HL,DE
	{
		word HLValue = M_RegisterHL;
		m_RegisterF.N = 0;
		m_RegisterF.C = (M_RegisterDE + HLValue) > 0xFFFF;
		m_RegisterF.H = ((M_RegisterDE & 0x0FFF) + (HLValue & 0x0FFF) > 0x0FFF);
		HLValue += M_RegisterDE;
		M_SetHL(HLValue);
	}
	break;
	case 0x1B: // DEC DE
	{
		word CurrentDEValue = M_RegisterDE;
		CurrentDEValue--;
		m_RegisterD = (CurrentDEValue & 0xFF00) >> 8;
		m_RegisterE = (CurrentDEValue & 0x00FF);
	}
	break;
	case 0x23: // INC HL
	{
		M_IncrementHL();
		break;
	}
	case 0x29: // ADD HL,HL
	{
		word CurrentHLValue = M_RegisterHL;
		m_RegisterF.N = 0;
		m_RegisterF.C = (CurrentHLValue + CurrentHLValue) > 0xFFFF;
		m_RegisterF.H = ((CurrentHLValue & 0x0FFF) + (CurrentHLValue & 0x0FFF) > 0x0FFF);
		CurrentHLValue += CurrentHLValue;
		M_SetHL(CurrentHLValue);
	}
	break;
	case 0x2B: // DEC HL
		M_DecrementHL();
		break;
	case 0x33: // INC SP
		m_StackPointer++;
		break;
	case 0x39: // ADD HL,SP
	{
		word CurrentHLValue = M_RegisterHL;
		m_RegisterF.N = 0;
		m_RegisterF.C = (CurrentHLValue + m_StackPointer) > 0xFFFF;
		m_RegisterF.H = ((CurrentHLValue & 0x0FFF) + (m_StackPointer & 0x0FFF) > 0x0FFF);
		CurrentHLValue += m_StackPointer;
		M_SetHL(CurrentHLValue);
	}
		break;
	case 0x3B: // DEC SP
		m_StackPointer--;
		break;
	case 0xE8: // ADD SP,r8
		m_RegisterF.Z = 0;
		m_RegisterF.N = 0;
		m_RegisterF.H = ((m_StackPointer & 0x000F) + (Op.Argument.sint8 & 0x0F)) > 0x0F;
		m_RegisterF.C = ((m_StackPointer & 0x00FF) + (Op.Argument.sint8 & 0xFF)) > 0xFF;
		m_StackPointer += Op.Argument.sint8;
		break;
	default:
		cout << "Instruction not defined: " << int(Op.OpCode) << endl;
		throw std::exception();
	}

	return Op.NumCycles[0];
}

int GBCPU::ExecuteRotShftBitOp(Instruction &Op)
{
	int cycles_used = 0;

	switch (Op.OpCode)
	{
	case 0x07: // RLCA
		if (m_RegisterA & BIT_7)
		{
			m_RegisterA = (m_RegisterA << 1) | BIT_0;
			m_RegisterF.C = 1;
		}
		else
		{
			m_RegisterA = (m_RegisterA << 1);
			m_RegisterF.C = 0;
		}
		
		m_RegisterF.Z = 0;
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
		break;
	case 0x0F: // RRCA
		if (m_RegisterA & BIT_0)
		{
			m_RegisterA = (m_RegisterA >> 1) | BIT_7;
			m_RegisterF.C = 1;
		}
		else
		{
			m_RegisterA = (m_RegisterA >> 1);
			m_RegisterF.C = 0;
		}

		m_RegisterF.Z = 0;
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
		break;
	case 0x17: // RLA
	{
		byte tempC = m_RegisterF.C;
		m_RegisterF.C = M_TestBit(m_RegisterA, BIT_7);
		m_RegisterA = (m_RegisterA << 1) | tempC;
		m_RegisterF.Z = 0;
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
	}
		break;
	case 0x1F: // RRA
	{
		byte OldBitZero = (m_RegisterA & BIT_0) ? 1 : 0;
		m_RegisterA >>= 1;
		m_RegisterA |= (m_RegisterF.C ? BIT_7 : 0);
		m_RegisterF.C = OldBitZero;
		m_RegisterF.Z = 0;
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
	}
		break;
	default:
		cout << "Instruction not defined: " << int(Op.OpCode) << endl;
		throw std::exception();
	}

	return cycles_used;
}

int GBCPU::ExecuteCBPrefixOp(Instruction &Op)
{
	int cycles_used = 0;

	switch (Op.OpCode)
	{
	case 0x00: // RLC B
		M_CB_RLC(m_RegisterB);
		break;
	case 0x01: // RLC C
		M_CB_RLC(m_RegisterC);
		break;
	case 0x02: // RLC D
		M_CB_RLC(m_RegisterD);
		break;
	case 0x03: // RLC E
		M_CB_RLC(m_RegisterE);
		break;
	case 0x04: // RLC H
		M_CB_RLC(m_RegisterH);
		break;
	case 0x05: // RLC L
		M_CB_RLC(m_RegisterL);
		break;
	case 0x06: // RLC (HL)
	{
		byte HLVal = m_MMU->ReadMemory8(M_RegisterHL);
		if (HLVal & BIT_7)
		{
			HLVal = (HLVal << 1) | 1;
			m_RegisterF.C = 1;
		}
		else
		{
			HLVal = (HLVal << 1);
			m_RegisterF.C = 0;
		}
		m_RegisterF.Z = (HLVal == 0);
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
		m_MMU->WriteMemory8(M_RegisterHL, HLVal);
	}
		break;
	case 0x07: // RLC A
		M_CB_RLC(m_RegisterA);
		break;
	case 0x08: // RRC B
		M_CB_RRC(m_RegisterB);
		break;
	case 0x09: // RRC C
		M_CB_RRC(m_RegisterC);
		break;
	case 0x0A: // RRC D
		M_CB_RRC(m_RegisterD);
		break;
	case 0x0B: // RRC E
		M_CB_RRC(m_RegisterE);
		break;
	case 0x0C: // RRC H
		M_CB_RRC(m_RegisterH);
		break;
	case 0x0D: // RRC L
		M_CB_RRC(m_RegisterL);
		break;
	case 0x0E: // RRC (HL)
	{
		byte HLVal = m_MMU->ReadMemory8(M_RegisterHL);

		if (HLVal & BIT_0)
		{
			HLVal = (HLVal >> 1) | BIT_7;
			m_RegisterF.C = 1;
		}
		else
		{
			HLVal = (HLVal >> 1);
			m_RegisterF.C = 0;
		}
		m_MMU->WriteMemory8(M_RegisterHL, HLVal);
		m_RegisterF.Z = (HLVal == 0);
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
	}
		break;
	case 0x0F: // RRC A
		M_CB_RRC(m_RegisterA);
		break;
	case 0x10: // RL B
		M_CB_RotateLeft(m_RegisterB);
		break;
	case 0x11: // RL C
		M_CB_RotateLeft(m_RegisterC);
		break;
	case 0x12: // RL D
		M_CB_RotateLeft(m_RegisterD);
		break;
	case 0x13: // RL E
		M_CB_RotateLeft(m_RegisterE);
		break;
	case 0x14: // RL H
		M_CB_RotateLeft(m_RegisterH);
		break;
	case 0x15: // RL L
		M_CB_RotateLeft(m_RegisterL);
		break;
	case 0x16: // RL (HL)
	{
		byte HLVal = m_MMU->ReadMemory8(M_RegisterHL);

		if (HLVal & BIT_7)
		{
			HLVal = (HLVal << 1) | m_RegisterF.C;
			m_RegisterF.C = 1; 
		}
		else
		{
			HLVal = (HLVal << 1) | m_RegisterF.C;
			m_RegisterF.C = 0;
		}
		m_MMU->WriteMemory8(M_RegisterHL, HLVal);
		m_RegisterF.Z = (HLVal == 0);
		m_RegisterF.H = 0;
		m_RegisterF.N = 0;
	}
		break;
	case 0x17: // RL A
		M_CB_RotateLeft(m_RegisterA);
		break;
	case 0x18: // RR B
		M_CB_RotateRight(m_RegisterB);
		break;
	case 0x19: // RR C
		M_CB_RotateRight(m_RegisterC);
		break;
	case 0x1A: // RR D
		M_CB_RotateRight(m_RegisterD);
		break;
	case 0x1B: // RR E
		M_CB_RotateRight(m_RegisterE);
		break;
	case 0x1C: // RR H
		M_CB_RotateRight(m_RegisterH);
		break;
	case 0x1D: // RR L
		M_CB_RotateRight(m_RegisterL);
		break;
	case 0x1E: // RR (HL)
	{
		byte HLVal = m_MMU->ReadMemory8(M_RegisterHL);

		if (HLVal & BIT_0)
		{
			HLVal = (HLVal >> 1) | (m_RegisterF.C ? BIT_7 : 0);
			m_RegisterF.C = 1;
		}
		else
		{
			HLVal = (HLVal >> 1) | (m_RegisterF.C ? BIT_7 : 0);
			m_RegisterF.C = 0;
		}
		m_MMU->WriteMemory8(M_RegisterHL, HLVal);
		m_RegisterF.Z = (HLVal == 0);
		m_RegisterF.H = 0;
		m_RegisterF.N = 0;
	}
		break;
	case 0x1F: // RR A
		M_CB_RotateRight(m_RegisterA);
		break;
	case 0x20: // SLA B
		M_CB_SLA(m_RegisterB);
		break;
	case 0x21: // SLA C
		M_CB_SLA(m_RegisterC);
		break;
	case 0x22: // SLA D
		M_CB_SLA(m_RegisterD);
		break;
	case 0x23: // SLA E
		M_CB_SLA(m_RegisterE);
		break;
	case 0x24: // SLA H
		M_CB_SLA(m_RegisterH);
		break;
	case 0x25: // SLA L
		M_CB_SLA(m_RegisterL);
		break;
	case 0x26: // SLA (HL)
	{
		byte HLVal = m_MMU->ReadMemory8(M_RegisterHL);
		m_RegisterF.C = M_TestBit(HLVal, BIT_7);
		HLVal <<= 1;
		m_RegisterF.Z = (HLVal == 0);
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
		m_MMU->WriteMemory8(M_RegisterHL, HLVal);
	}
		break;
	case 0x27: // SLA A
		M_CB_SLA(m_RegisterA);
		break;
	case 0x28: // SRA B
		//m_Emulator->SetDebugMode(DEBUG_STEP);
		M_CB_SRA(m_RegisterB);
		break;
	case 0x29: // SRA C
		M_CB_SRA(m_RegisterC);
		break;
	case 0x2A: // SRA D
		M_CB_SRA(m_RegisterD);
		break;
	case 0x2B: // SRA E
		M_CB_SRA(m_RegisterE);
		break;
	case 0x2C: // SRA H
		M_CB_SRA(m_RegisterH);
		break;
	case 0x2D: // SRA L
		M_CB_SRA(m_RegisterL);
		break;
	case 0x2E: // SRA (HL)
	{
		byte HLVal = m_MMU->ReadMemory8(M_RegisterHL);
		m_RegisterF.C = M_TestBit(HLVal, BIT_0);
		HLVal = (HLVal >> 1) | (HLVal & 0x80);
		m_RegisterF.Z = (HLVal == 0);
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
		m_MMU->WriteMemory8(M_RegisterHL,HLVal);
	}
		break;
	case 0x2F: // SRA A
		M_CB_SRA(m_RegisterA);
		break;
	case 0x30: // SWAP B
		M_CB_Swap(m_RegisterB);
		break;
	case 0x31: // SWAP C
		M_CB_Swap(m_RegisterC);
		break;
	case 0x32: // SWAP D
		M_CB_Swap(m_RegisterD);
		break;
	case 0x33: // SWAP E
		M_CB_Swap(m_RegisterE);
		break;
	case 0x34: // SWAP H
		M_CB_Swap(m_RegisterH);
		break;
	case 0x35: // SWAP L
		M_CB_Swap(m_RegisterL);
		break;
	case 0x36: // SWAP (HL)
	{
		byte HLVal = m_MMU->ReadMemory8(M_RegisterHL);
		HLVal = (HLVal << 4 | HLVal >> 4) & 0xFF;
		m_MMU->WriteMemory8(M_RegisterHL, HLVal);
		m_RegisterF.Z = (HLVal == 0);
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
		m_RegisterF.C = 0;
	}
		break;
	case 0x37: // SWAP A
		M_CB_Swap(m_RegisterA);
		break;
	case 0x38: // SRL B
		M_CB_SRL(m_RegisterB);
		break;
	case 0x39: // SRL C
		M_CB_SRL(m_RegisterC);
		break;
	case 0x3A: // SRL D
		M_CB_SRL(m_RegisterD);
		break;
	case 0x3B: // SRL E
		M_CB_SRL(m_RegisterE);
		break;
	case 0x3C: // SRL H
		M_CB_SRL(m_RegisterH);
		break;
	case 0x3D: // SRL L
		M_CB_SRL(m_RegisterL);
		break;
	case 0x3E: // SRL (HL)
	{
		byte HLVal = m_MMU->ReadMemory8(M_RegisterHL);
		m_RegisterF.C = (HLVal & BIT_0) ? 1 : 0;
		HLVal >>= 1;
		m_MMU->WriteMemory8(M_RegisterHL, HLVal);
		m_RegisterF.Z = (HLVal == 0);
		m_RegisterF.N = 0;
		m_RegisterF.H = 0;
	}
		break;
	case 0x3F: // SRL A
		M_CB_SRL(m_RegisterA);
		break;
	case 0x40: // BIT 0,B
		M_CB_TestBit(m_RegisterB, BIT_0);
		break;
	case 0x41: // BIT 0,C
		M_CB_TestBit(m_RegisterC, BIT_0);
		break;
	case 0x42: // BIT 0,D
		M_CB_TestBit(m_RegisterD, BIT_0);
		break;
	case 0x43: // BIT 0,E
		M_CB_TestBit(m_RegisterE, BIT_0);
		break;
	case 0x44: // BIT 0,H
		M_CB_TestBit(m_RegisterH, BIT_0);
		break;
	case 0x45: // BIT 0,L
		M_CB_TestBit(m_RegisterL, BIT_0);
		break;
	case 0x46: // BIT 0,(HL)
		M_CB_TestBit(m_MMU->ReadMemory8(M_RegisterHL), BIT_0);
		break;
	case 0x47: // BIT 0,A
		M_CB_TestBit(m_RegisterA, BIT_0);
		break;
	case 0x48: // BIT 1,B
		M_CB_TestBit(m_RegisterB, BIT_1);
		break;
	case 0x49: // BIT 1,C
		M_CB_TestBit(m_RegisterC, BIT_1);
		break;
	case 0x4A: // BIT 1,D
		M_CB_TestBit(m_RegisterD, BIT_1);
		break;
	case 0x4B: // BIT 1,E
		M_CB_TestBit(m_RegisterE, BIT_1);
		break;
	case 0x4C: // BIT 1,H
		M_CB_TestBit(m_RegisterH, BIT_1);
		break;
	case 0x4D: // BIT 1,L
		M_CB_TestBit(m_RegisterL, BIT_1);
		break;
	case 0x4E: // BIT 1,(HL)
		M_CB_TestBit(m_MMU->ReadMemory8(M_RegisterHL), BIT_1);
		break;
	case 0x4F: // BIT 1,A
		M_CB_TestBit(m_RegisterA, BIT_1);
		break;
	case 0x50: // BIT 2,B
		M_CB_TestBit(m_RegisterB, BIT_2);
		break;
	case 0x51: // BIT 2,C
		M_CB_TestBit(m_RegisterC, BIT_2);
		break;
	case 0x52: // BIT 2,D
		M_CB_TestBit(m_RegisterD, BIT_2);
		break;
	case 0x53: // BIT 2,E
		M_CB_TestBit(m_RegisterE, BIT_2);
		break;
	case 0x54: // BIT 2,H
		M_CB_TestBit(m_RegisterH, BIT_2);
		break;
	case 0x55: // BIT 2,L
		M_CB_TestBit(m_RegisterL, BIT_2);
		break;
	case 0x56: // BIT 2,(HL)
		M_CB_TestBit(m_MMU->ReadMemory8(M_RegisterHL), BIT_2);
		break;
	case 0x57: // BIT 2,A
		M_CB_TestBit(m_RegisterA, BIT_2);
		break;
	case 0x58: // BIT 3,B
		M_CB_TestBit(m_RegisterB, BIT_3);
		break;
	case 0x59: // BIT 3,C
		M_CB_TestBit(m_RegisterC, BIT_3);
		break;
	case 0x5A: // BIT 3,D
		M_CB_TestBit(m_RegisterD, BIT_3);
		break;
	case 0x5B: // BIT 3,E
		M_CB_TestBit(m_RegisterE, BIT_3);
		break;
	case 0x5C: // BIT 3,H
		M_CB_TestBit(m_RegisterH, BIT_3);
		break;
	case 0x5D: // BIT 3,L
		M_CB_TestBit(m_RegisterL, BIT_3);
		break;
	case 0x5E: // BIT 3,(HL)
		M_CB_TestBit(m_MMU->ReadMemory8(M_RegisterHL), BIT_3);
		break;
	case 0x5F: // BIT 3,A
		M_CB_TestBit(m_RegisterA, BIT_3);
		break;
	case 0x60: // BIT 4,B
		M_CB_TestBit(m_RegisterB, BIT_4);
		break;
	case 0x61: // BIT 4,C
		M_CB_TestBit(m_RegisterC, BIT_4);
		break;
	case 0x62: // BIT 4,D
		M_CB_TestBit(m_RegisterD, BIT_4);
		break;
	case 0x63: // BIT 4,E
		M_CB_TestBit(m_RegisterE, BIT_4);
		break;
	case 0x64: // BIT 4,H
		M_CB_TestBit(m_RegisterH, BIT_4);
		break;
	case 0x65: // BIT 4,L
		M_CB_TestBit(m_RegisterL, BIT_4);
		break;
	case 0x66: // BIT 4,(HL)
		M_CB_TestBit(m_MMU->ReadMemory8(M_RegisterHL), BIT_4);
		break;
	case 0x67: // BIT 4,A
		M_CB_TestBit(m_RegisterA, BIT_4);
		break;
	case 0x68: // BIT 5,B
		M_CB_TestBit(m_RegisterB, BIT_5);
		break;
	case 0x69: // BIT 5,C
		M_CB_TestBit(m_RegisterC, BIT_5);
		break;
	case 0x6A: // BIT 5,D
		M_CB_TestBit(m_RegisterD, BIT_5);
		break;
	case 0x6B: // BIT 5,E
		M_CB_TestBit(m_RegisterE, BIT_5);
		break;
	case 0x6C: // BIT 5,H
		M_CB_TestBit(m_RegisterH, BIT_5);
		break;
	case 0x6D: // BIT 5,L
		M_CB_TestBit(m_RegisterL, BIT_5);
		break;
	case 0x6E: // BIT 5,(HL)
		M_CB_TestBit(m_MMU->ReadMemory8(M_RegisterHL), BIT_5);
		break;
	case 0x6F: // BIT 5,A
		M_CB_TestBit(m_RegisterA, BIT_5);
		break;
	case 0x70: // BIT 6,B
		M_CB_TestBit(m_RegisterB, BIT_6);
		break;
	case 0x71: // BIT 6,C
		M_CB_TestBit(m_RegisterC, BIT_6);
		break;
	case 0x72: // BIT 6,D
		M_CB_TestBit(m_RegisterD, BIT_6);
		break;
	case 0x73: // BIT 6,E
		M_CB_TestBit(m_RegisterE, BIT_6);
		break;
	case 0x74: // BIT 6,H
		M_CB_TestBit(m_RegisterH, BIT_6);
		break;
	case 0x75: // BIT 6,L
		M_CB_TestBit(m_RegisterL, BIT_6);
		break;
	case 0x76: // BIT 6,(HL)
		M_CB_TestBit(m_MMU->ReadMemory8(M_RegisterHL), BIT_6);
		break;
	case 0x77: // BIT 6,A
		M_CB_TestBit(m_RegisterA, BIT_6);
		break;
	case 0x78: // BIT 7,B
		M_CB_TestBit(m_RegisterB, BIT_7);
		break;
	case 0x79: // BIT 7,C
		M_CB_TestBit(m_RegisterC, BIT_7);
		break;
	case 0x7A: // BIT 7,D
		M_CB_TestBit(m_RegisterD, BIT_7);
		break;
	case 0x7B: // BIT 7,E
		M_CB_TestBit(m_RegisterE, BIT_7);
		break;
	case 0x7C: // BIT 7,H
		M_CB_TestBit(m_RegisterH, BIT_7);
		break;
	case 0x7D: // BIT 7,L
		M_CB_TestBit(m_RegisterL, BIT_7);
		break;
	case 0x7E: // BIT 7,(HL)
		M_CB_TestBit(m_MMU->ReadMemory8(M_RegisterHL), BIT_7);
		break;
	case 0x7F: // BIT 7,A
		M_CB_TestBit(m_RegisterA, BIT_7);
		break;
	case 0x80: // RES 0,B
		M_ResetRegBit(m_RegisterB, BIT_0);
		break;
	case 0x81: // RES 0,C
		M_ResetRegBit(m_RegisterC, BIT_0);
		break;
	case 0x82: // RES 0,D
		M_ResetRegBit(m_RegisterD, BIT_0);
		break;
	case 0x83: // RES 0,E
		M_ResetRegBit(m_RegisterE, BIT_0);
		break;
	case 0x84: // RES 0,H
		M_ResetRegBit(m_RegisterH, BIT_0);
		break;
	case 0x85: // RES 0,L
		M_ResetRegBit(m_RegisterL, BIT_0);
		break;
	case 0x86: // RES 0,(HL)
		M_ResetBitAtHL(BIT_0);
		break;
	case 0x87: // RES 0,A
		M_ResetRegBit(m_RegisterA, BIT_0);
		break;
	case 0x88: // RES 1,B
		M_ResetRegBit(m_RegisterB, BIT_1);
		break;
	case 0x89: // RES 1,C
		M_ResetRegBit(m_RegisterC, BIT_1);
		break;
	case 0x8A: // RES 1,D
		M_ResetRegBit(m_RegisterD, BIT_1);
		break;
	case 0x8B: // RES 1,E
		M_ResetRegBit(m_RegisterE, BIT_1);
		break;
	case 0x8C: // RES 1,H
		M_ResetRegBit(m_RegisterH, BIT_1);
		break;
	case 0x8D: // RES 1,L
		M_ResetRegBit(m_RegisterL, BIT_1);
		break;
	case 0x8E: // RES 1,(HL)
		M_ResetBitAtHL(BIT_1);
		break;
	case 0x8F: // RES 1,A
		M_ResetRegBit(m_RegisterA, BIT_1);
		break;
	case 0x90: // RES 2,B
		M_ResetRegBit(m_RegisterB, BIT_2);
		break;
	case 0x91: // RES 2,C
		M_ResetRegBit(m_RegisterC, BIT_2);
		break;
	case 0x92: // RES 2,D
		M_ResetRegBit(m_RegisterD, BIT_2);
		break;
	case 0x93: // RES 2,E
		M_ResetRegBit(m_RegisterE, BIT_2);
		break;
	case 0x94: // RES 2,H
		M_ResetRegBit(m_RegisterH, BIT_2);
		break;
	case 0x95: // RES 2,L
		M_ResetRegBit(m_RegisterL, BIT_2);
		break;
	case 0x96: // RES 2,(HL)
		M_ResetBitAtHL(BIT_2);
		break;
	case 0x97: // RES 2,A
		M_ResetRegBit(m_RegisterA, BIT_2);
		break;
	case 0x98: // RES 3,B
		M_ResetRegBit(m_RegisterB, BIT_3);
		break;
	case 0x99: // RES 3,C
		M_ResetRegBit(m_RegisterC, BIT_3);
		break;
	case 0x9A: // RES 3,D
		M_ResetRegBit(m_RegisterD, BIT_3);
		break;
	case 0x9B: // RES 3,E
		M_ResetRegBit(m_RegisterE, BIT_3);
		break;
	case 0x9C: // RES 3,H
		M_ResetRegBit(m_RegisterH, BIT_3);
		break;
	case 0x9D: // RES 3,L
		M_ResetRegBit(m_RegisterL, BIT_3);
		break;
	case 0x9E: // RES 3,(HL)
		M_ResetBitAtHL(BIT_3);
		break;
	case 0x9F: // RES 3,A
		M_ResetRegBit(m_RegisterA, BIT_3);
		break;
	case 0xA0: // RES 4,B
		M_ResetRegBit(m_RegisterB, BIT_4);
		break;
	case 0xA1: // RES 4,C
		M_ResetRegBit(m_RegisterC, BIT_4);
		break;
	case 0xA2: // RES 4,D
		M_ResetRegBit(m_RegisterD, BIT_4);
		break;
	case 0xA3: // RES 4,E
		M_ResetRegBit(m_RegisterE, BIT_4);
		break;
	case 0xA4: // RES 4,H
		M_ResetRegBit(m_RegisterH, BIT_4);
		break;
	case 0xA5: // RES 4,L
		M_ResetRegBit(m_RegisterL, BIT_4);
		break;
	case 0xA6: // RES 4,(HL)
		M_ResetBitAtHL(BIT_4);
		break;
	case 0xA7: // RES 4,A
		M_ResetRegBit(m_RegisterA, BIT_4);
		break;
	case 0xA8: // RES 5,B
		M_ResetRegBit(m_RegisterB, BIT_5);
		break;
	case 0xA9: // RES 5,C
		M_ResetRegBit(m_RegisterC, BIT_5);
		break;
	case 0xAA: // RES 5,D
		M_ResetRegBit(m_RegisterD, BIT_5);
		break;
	case 0xAB: // RES 5,E
		M_ResetRegBit(m_RegisterE, BIT_5);
		break;
	case 0xAC: // RES 5,H
		M_ResetRegBit(m_RegisterH, BIT_5);
		break;
	case 0xAD: // RES 5,L
		M_ResetRegBit(m_RegisterL, BIT_5);
		break;
	case 0xAE: // RES 5,(HL)
		M_ResetBitAtHL(BIT_5);
		break;
	case 0xAF: // RES 5,A
		M_ResetRegBit(m_RegisterA, BIT_5);
		break;
	case 0xB0: // RES 6,B
		M_ResetRegBit(m_RegisterB, BIT_6);
		break;
	case 0xB1: // RES 6,C
		M_ResetRegBit(m_RegisterC, BIT_6);
		break;
	case 0xB2: // RES 6,D
		M_ResetRegBit(m_RegisterD, BIT_6);
		break;
	case 0xB3: // RES 6,E
		M_ResetRegBit(m_RegisterE, BIT_6);
		break;
	case 0xB4: // RES 6,H
		M_ResetRegBit(m_RegisterH, BIT_6);
		break;
	case 0xB5: // RES 6,L
		M_ResetRegBit(m_RegisterL, BIT_6);
		break;
	case 0xB6: // RES 6,(HL)
		M_ResetBitAtHL(BIT_6);
		break;
	case 0xB7: // RES 6,A
		M_ResetRegBit(m_RegisterA, BIT_6);
		break;
	case 0xB8: // RES 7,B
		M_ResetRegBit(m_RegisterB, BIT_7);
		break;
	case 0xB9: // RES 7,C
		M_ResetRegBit(m_RegisterC, BIT_7);
		break;
	case 0xBA: // RES 7,D
		M_ResetRegBit(m_RegisterD, BIT_7);
		break;
	case 0xBB: // RES 7,E
		M_ResetRegBit(m_RegisterE, BIT_7);
		break;
	case 0xBC: // RES 7,H
		M_ResetRegBit(m_RegisterH, BIT_7);
		break;
	case 0xBD: // RES 7,L
		M_ResetRegBit(m_RegisterL, BIT_7);
		break;
	case 0xBE: // RES 7,(HL)
		M_ResetBitAtHL(BIT_7);
		break;
	case 0xBF: // RES 7,A
		M_ResetRegBit(m_RegisterA, BIT_7);
		break;
	case 0xC0: // SET 0,B
		M_SetRegBit(m_RegisterB, BIT_0);
		break;
	case 0xC1: // SET 0,C
		M_SetRegBit(m_RegisterC, BIT_0);
		break;
	case 0xC2: // SET 0,D
		M_SetRegBit(m_RegisterD, BIT_0);
		break;
	case 0xC3: // SET 0,E
		M_SetRegBit(m_RegisterE, BIT_0);
		break;
	case 0xC4: // SET 0,H
		M_SetRegBit(m_RegisterH, BIT_0);
		break;
	case 0xC5: // SET 0,L
		M_SetRegBit(m_RegisterL, BIT_0);
		break;
	case 0xC6: // SET 0,(HL)
		M_SetBitAtHL(BIT_0);
		break;
	case 0xC7: // SET 0,A
		M_SetRegBit(m_RegisterA, BIT_0);
		break;
	case 0xC8: // SET 1,B
		M_SetRegBit(m_RegisterB, BIT_1);
		break;
	case 0xC9: // SET 1,C
		M_SetRegBit(m_RegisterC, BIT_1);
		break;
	case 0xCA: // SET 1,D
		M_SetRegBit(m_RegisterD, BIT_1);
		break;
	case 0xCB: // SET 1,E
		M_SetRegBit(m_RegisterE, BIT_1);
		break;
	case 0xCC: // SET 1,H
		M_SetRegBit(m_RegisterH, BIT_1);
		break;
	case 0xCD: // SET 1,L
		M_SetRegBit(m_RegisterL, BIT_1);
		break;
	case 0xCE: // SET 1,(HL)
		M_SetBitAtHL(BIT_1);
		break;
	case 0xCF: // SET 1,A
		M_SetRegBit(m_RegisterA, BIT_1);
		break;
	case 0xD0: // SET 2,B
		M_SetRegBit(m_RegisterB, BIT_2);
		break;
	case 0xD1: // SET 2,C
		M_SetRegBit(m_RegisterC, BIT_2);
		break;
	case 0xD2: // SET 2,D
		M_SetRegBit(m_RegisterD, BIT_2);
		break;
	case 0xD3: // SET 2,E
		M_SetRegBit(m_RegisterE, BIT_2);
		break;
	case 0xD4: // SET 2,H
		M_SetRegBit(m_RegisterH, BIT_2);
		break;
	case 0xD5: // SET 2,L
		M_SetRegBit(m_RegisterL, BIT_2);
		break;
	case 0xD6: // SET 2,(HL)
		M_SetBitAtHL(BIT_2);
		break;
	case 0xD7: // SET 2,A
		M_SetRegBit(m_RegisterA, BIT_2);
		break;
	case 0xD8: // SET 3,B
		M_SetRegBit(m_RegisterB, BIT_3);
		break;
	case 0xD9: // SET 3,C
		M_SetRegBit(m_RegisterC, BIT_3);
		break;
	case 0xDA: // SET 3,D
		M_SetRegBit(m_RegisterD, BIT_3);
		break;
	case 0xDB: // SET 3,E
		M_SetRegBit(m_RegisterE, BIT_3);
		break;
	case 0xDC: // SET 3,H
		M_SetRegBit(m_RegisterH, BIT_3);
		break;
	case 0xDD: // SET 3,L
		M_SetRegBit(m_RegisterL, BIT_3);
		break;
	case 0xDE: // SET 3,(HL)
		M_SetBitAtHL(BIT_3);
		break;
	case 0xDF: // SET 3,A
		M_SetRegBit(m_RegisterA, BIT_3);
		break;
	case 0xE0: // SET 4,B
		M_SetRegBit(m_RegisterB, BIT_4);
		break;
	case 0xE1: // SET 4,C
		M_SetRegBit(m_RegisterC, BIT_4);
		break;
	case 0xE2: // SET 4,D
		M_SetRegBit(m_RegisterD, BIT_4);
		break;
	case 0xE3: // SET 4,E
		M_SetRegBit(m_RegisterE, BIT_4);
		break;
	case 0xE4: // SET 4,H
		M_SetRegBit(m_RegisterH, BIT_4);
		break;
	case 0xE5: // SET 4,L
		M_SetRegBit(m_RegisterL, BIT_4);
		break;
	case 0xE6: // SET 4,(HL)
		M_SetBitAtHL(BIT_4);
		break;
	case 0xE7: // SET 4,A
		M_SetRegBit(m_RegisterA, BIT_4);
		break;
	case 0xE8: // SET 5,B
		M_SetRegBit(m_RegisterB, BIT_5);
		break;
	case 0xE9: // SET 5,C
		M_SetRegBit(m_RegisterC, BIT_5);
		break;
	case 0xEA: // SET 5,D
		M_SetRegBit(m_RegisterD, BIT_5);
		break;
	case 0xEB: // SET 5,E
		M_SetRegBit(m_RegisterE, BIT_5);
		break;
	case 0xEC: // SET 5,H
		M_SetRegBit(m_RegisterH, BIT_5);
		break;
	case 0xED: // SET 5,L
		M_SetRegBit(m_RegisterL, BIT_5);
		break;
	case 0xEE: // SET 5,(HL)
		M_SetBitAtHL(BIT_5);
		break;
	case 0xEF: // SET 5,A
		M_SetRegBit(m_RegisterA, BIT_5);
		break;
	case 0xF0: // SET 6,B
		M_SetRegBit(m_RegisterB, BIT_6);
		break;
	case 0xF1: // SET 6,C
		M_SetRegBit(m_RegisterC, BIT_6);
		break;
	case 0xF2: // SET 6,D
		M_SetRegBit(m_RegisterD, BIT_6);
		break;
	case 0xF3: // SET 6,E
		M_SetRegBit(m_RegisterE, BIT_6);
		break;
	case 0xF4: // SET 6,H
		M_SetRegBit(m_RegisterH, BIT_6);
		break;
	case 0xF5: // SET 6,L
		M_SetRegBit(m_RegisterL, BIT_6);
		break;
	case 0xF6: // SET 6,(HL)
		M_SetBitAtHL(BIT_6);
		break;
	case 0xF7: // SET 6,A
		M_SetRegBit(m_RegisterA, BIT_6);
		break;
	case 0xF8: // SET 7,B
		M_SetRegBit(m_RegisterB, BIT_7);
		break;
	case 0xF9: // SET 7,C
		M_SetRegBit(m_RegisterC, BIT_7);
		break;
	case 0xFA: // SET 7,D
		M_SetRegBit(m_RegisterD, BIT_7);
		break;
	case 0xFB: // SET 7,E
		M_SetRegBit(m_RegisterE, BIT_7);
		break;
	case 0xFC: // SET 7,H
		M_SetRegBit(m_RegisterH, BIT_7);
		break;
	case 0xFD: // SET 7,L
		M_SetRegBit(m_RegisterL, BIT_7);
		break;
	case 0xFE: // SET 7,(HL)
		M_SetBitAtHL(BIT_7);
		break;
	case 0xFF: // SET 7,A
		M_SetRegBit(m_RegisterA, BIT_7);
		break;
	default:
		cout << "Instruction not defined: " << int(Op.OpCode) << endl;
		throw std::exception();
	}

	return Op.NumCycles[0];
}