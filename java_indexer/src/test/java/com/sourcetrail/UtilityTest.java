package com.sourcetrail;

import static org.junit.jupiter.api.Assertions.*;
import org.junit.jupiter.api.Test;
import org.apache.commons.lang3.SystemUtils;

class UtilityTest {

	@Test
	void testGetClassPathSeparator()
	{
		if (SystemUtils.IS_OS_WINDOWS)
			assertEquals(";", Utility.getClassPathSeparator());
		else
			assertEquals(":", Utility.getClassPathSeparator());
	}

}
