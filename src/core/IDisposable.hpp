#pragma once

#include "../common.h"

/**
 * Represents an object that can be disposed. So things can explicitly close resources before the destructor kicks in.
 */
interface IDisposable {
	virtual void Dispose() abstract;
};
