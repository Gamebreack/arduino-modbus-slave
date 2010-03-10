#include <modbusRegBank.h>

modbusRegBank::modbusRegBank(void)
{
	_digRegs		= 0;
	_lastDigReg		= 0;
	_anaRegs		= 0;
	_lastAnaReg		= 0;
	_eepromStart	= EEPROMSTART;
	_eepromEnd		= EEPROMSTART + EEPROMLEN - 1;
}


void modbusRegBank::add(word addr)
{
	if(addr<20000)
	{
		modbusDigReg *temp;

		temp = (modbusDigReg *) malloc(sizeof(modbusDigReg));
		temp->address = addr;
		temp->value		= 0;
		temp->next		= 0;

		if(_digRegs == 0)
		{
			_digRegs = temp;
			_lastDigReg = _digRegs;
		}
		else
		{
			//Assign the last register's next pointer to temp;
			_lastDigReg->next = temp;
			//then make temp the last register in the list.
			_lastDigReg = temp;
		}
		return;
	}	
	else if((addr < _eepromStart) || (addr > _eepromEnd))  //Create new Reg if not in EEPROM
	{
		modbusAnaReg *temp;

		temp = (modbusAnaReg *) malloc(sizeof(modbusAnaReg));
		temp->address = addr;
		temp->value = 0;
		temp->next = 0;

		if(_anaRegs == 0)
		{
			_anaRegs = temp;
			_lastAnaReg = _anaRegs;
		}
		else
		{
			_lastAnaReg->next = temp;
			_lastAnaReg = temp;
		}
		return;
	}
}

word modbusRegBank::get(word addr)
{
	if(addr < 20000)
	{
		modbusDigReg * regPtr;
		regPtr = (modbusDigReg *) this->search(addr);
		if(regPtr)
			return(regPtr->value);
		else
			return(NULL);	
	}
	else if((addr < _eepromStart) || (addr > _eepromEnd))
	{
		modbusAnaReg * regPtr;
		regPtr = (modbusAnaReg *) this->search(addr);
		if(regPtr)
			return(regPtr->value);
		else
			return(NULL);	
	}
	else //return the EEPROM value
	{
		return(EEPROM.read(addr - _eepromStart));
	}
}

void modbusRegBank::set(word addr, word value)
{
	//for digital data
	if(addr < 20000)
	{
		modbusDigReg * regPtr;
		//search for the register address
		regPtr = (modbusDigReg *) this->search(addr);
		//if a pointer was returned the set the register value to true if value is non zero
		if(regPtr)
			if(value)
				regPtr->value = 0xFF;
			else
				regPtr->value = 0x00;
	}
	else if ((addr < _eepromStart) || (addr > _eepromEnd))
	{
		modbusAnaReg * regPtr;
		//search for the register address
		regPtr = (modbusAnaReg *) this->search(addr);
		//if found then assign the register value to the new value.
		if(regPtr)
			regPtr->value = value;
	}
	else //EEPROM WRITE
	{
		//No search needed. Write to EEPROM after masking off the HSB
		EEPROM.write(addr - _eepromStart, (byte)(value & 0x00FF));
	}
}

void * modbusRegBank::search(word addr)
{
	//if the requested address is 0-19999 
	//use a digital register pointer assigned to the first digital register
	//else use a analog register pointer assigned the first analog register

	if(addr < 20000)
	{
		modbusDigReg *regPtr = _digRegs;

		//if there is no register configured, bail
		if(regPtr == 0)
			return(0);

		//scan through the linked list until the end of the list or the register is found.
		//return the pointer.
		do
		{
			if(regPtr->address == addr)
				return(regPtr);
			regPtr = regPtr->next;
		}
		while(regPtr);
	}
	else
	{
		modbusAnaReg *regPtr = _anaRegs;

		//if there is no register configured, bail
		if(regPtr == 0)
			return(0);

		//scan through the linked list until the end of the list or the register is found.
		//return the pointer.
		do
		{
			if(regPtr->address == addr)
				return(regPtr);
			regPtr = regPtr->next;
		}
		while(regPtr);
	}
	return(0);
}


