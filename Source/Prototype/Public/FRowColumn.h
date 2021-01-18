#pragma once

// a convenience structure for holding two integers, a row and column
struct FRowColumn
{
	int32 Row;
	int32 Column;

	FORCEINLINE FRowColumn(int32 inRow, int32 inColumn) : Row(inRow), Column(inColumn) {}

	FORCEINLINE bool operator==(const FRowColumn &input) const
	{
		return Row == input.Row && Column == input.Column;
	}
};
