#include "Cartridge.h"
#include <fstream>

using namespace std;

Cartridge::Cartridge(EmulatorEngine *Emulator)
{
	m_Emulator = Emulator;
}

bool Cartridge::Initialize(wxString RomFilePath)
{
	ReadRomFile(RomFilePath);
	ReadCartAttributes();
	InitMBC();

	return true;
}

byte Cartridge::ReadMemory8(word Address)
{
	return m_MBC->ReadMemory8(Address);
}

void Cartridge::WriteMemory8(word Address, byte Value)
{
	return m_MBC->WriteMemory8(Address, Value);
}

void Cartridge::ReadCartAttributes()
{
	m_CartAttrs.CartType = (CartridgeType)m_RomData[CART_TYPE_ADDR];
	m_CartAttrs.RomSize = m_RomData[CART_ROM_SIZE_ADDR];
	m_CartAttrs.RamSize = RamSizeMap[m_RomData[CART_RAM_SIZE_ADDR]];
}

void Cartridge::InitMBC()
{
	if (m_MBC != nullptr)
	{
		delete m_MBC;
		m_MBC = nullptr;
	}

	switch (m_CartAttrs.CartType)
	{
	case CART_ROM_ONLY:
	case CART_ROM_RAM:
	case CART_ROM_RAM_BAT:
		m_MBC = new MBC0(m_CartAttrs, m_RomData);
	case CART_MBC1:
	case CART_MBC1_RAM:
	case CART_MBC1_RAM_BAT:
		m_MBC = new MBC1(m_CartAttrs, m_RomData);
		break;
	case CART_MBC3_TIM_BAT:
	case CART_MBC3_TIM_RAM_BAT:
	case CART_MBC3:
	case CART_MBC3_RAM:
	case CART_MBC3_RAM_BAT:
		m_MBC = new MBC3(m_CartAttrs, m_RomData);
		break;
	default:
		cout << "Unimplemented MBC!" << endl;
		throw exception();
	}

}

bool Cartridge::ReadRomFile(wxString RomFilePath)
{
	ifstream RomFile;
	size_t RomSize = 0;
	bool RomFileRead = false;

	if (m_RomData != nullptr)
	{
		delete m_RomData;
		m_RomData = nullptr;
	}

	RomFile.open(RomFilePath.ToUTF8(), ios::in | ios::binary);
	if (RomFile.is_open())
	{
		RomFileRead = true;
		RomFile.seekg(0, ios::end);
		RomSize = RomFile.tellg();
		RomFile.seekg(0, ios::beg);
		m_RomData = new byte[RomSize];
		RomFile.read((char *)m_RomData, RomSize);
	}

	return RomFileRead;
}

byte Cartridge::GetRomBankNumber()
{
	byte BankNumber = 0;
	
	if (m_MBC != nullptr)
	{
		BankNumber = m_MBC->GetRomBankNumber();
	}

	return BankNumber;
}

byte Cartridge::GetRamBankNumber()
{
	byte BankNumber = 0;

	if (m_MBC != nullptr)
	{
		BankNumber = m_MBC->GetRamBankNumber();
	}

	return BankNumber;
}

Cartridge::~Cartridge()
{
	delete m_MBC;
	delete m_RomData;
}
