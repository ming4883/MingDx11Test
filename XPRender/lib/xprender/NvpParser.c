#include "NVPParser.h"

void XprNvpParser_init(XprNvpParser* self, const char* str)
{
	self->mStr = self->mPos = (char*)str;
}

XprBool advancePos(char** pos)
{
	if(**pos != '\0') {
		++(*pos);
		return XprTrue;
	}
	return XprFalse;
}

static void skipSeps(char** pos)
{
	static const char cSeps[] = ",; \t\n\r";
	static const size_t cCount = sizeof(cSeps) - 1;
	size_t i;

	for(i=cCount; i--;) {
		if(**pos != cSeps[i])
			continue;
		i = cCount;
		if(!advancePos(pos))
			return;
	}
}

void skipNonSeps(char** pos)
{
	static const char cSeps[] = "=,; \t\n\r";
	static const size_t cCount = sizeof(cSeps) - 1;

	size_t i;
	do {
		for(i=cCount; i--;) {
			if(**pos == cSeps[i])
				return;
		}
	} while(advancePos(pos));
}

XprBool XprNvpParser_next(XprNvpParser* self, const char** name, const char** value)
{
	static const char cQuots[] = "'\"";
	char* name_;

	// Get the name
	skipSeps(&self->mPos);
	name_ = self->mPos;
	skipNonSeps(&self->mPos);

	if(*self->mPos != '=' && advancePos(&self->mPos)) {
		*(self->mPos-1) = '\0';
		skipSeps(&self->mPos);
	}

	// Should be '='
	if(*self->mPos != '=')
		return XprFalse;
	*(self->mPos++) = '\0';

	// Get the value
	skipSeps(&self->mPos);
	// Get quoted string
	if(*self->mPos == cQuots[0] || *self->mPos == cQuots[1]) {
		char quot = *self->mPos;
		*value = self->mPos + 1;
		while(advancePos(&self->mPos) && *self->mPos != quot);
	} else {
		*value = self->mPos;
		skipNonSeps(&self->mPos);
	}

	if(advancePos(&self->mPos))
		*(self->mPos-1) = '\0';

	*name = name_;
	return XprTrue;
}