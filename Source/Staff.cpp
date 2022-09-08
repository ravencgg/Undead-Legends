#include "Staff.h"

void Character::newStaff(Staff* newStaff) {
	delete staff;
	staff = newStaff;
}