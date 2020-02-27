#pragma once

struct RemoveCopies
{
	RemoveCopies(const RemoveCopies& copy) = delete;
	RemoveCopies& operator=(const RemoveCopies& copy) = delete;
};

struct RemoveMoves
{
	RemoveMoves(RemoveMoves&& move) = delete;
	RemoveMoves& operator=(RemoveMoves&& move) = delete;
};
