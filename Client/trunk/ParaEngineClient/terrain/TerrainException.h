#pragma once
#include <string>
#include <iostream>

namespace ParaTerrain
{
	/// \brief An exception that is thrown by Demeter objects to 
	/// indicate fatal errors that your application should handle.
	class TerrainException
	{
	public:
		TerrainException(const std::string & errorMessage)
		{
			m_ErrorMessage = "TERRAIN ERROR: ";
			m_ErrorMessage.append(errorMessage);
		}

		 ~TerrainException()
		{
		}

		/// \brief Gets a human readable string describing the error condition.
		const char *GetErrorMessage()
		{
			return m_ErrorMessage.c_str();
		}
	private:
		std::string m_ErrorMessage;
	};
}
