package com.tatfook.paracraft;
import com.tatfook.paracraft.ParaEngineLuaJavaBridge;
import java.lang.Thread;

public class LuaFunction
{
	private int m_id = -1;
	private long m_threadId;
	private Runnable m_releaseRunable = null;

	public LuaFunction(int id)
	{
		m_id = id;

		m_threadId = Thread.currentThread().getId();

		m_releaseRunable = new Runnable() {

			@Override
			public void run() {
				final int id = m_id;
				final long threadId = m_threadId;

				if (id != -1)
				{
					AppActivity mainApp = (AppActivity)AppActivity.getContext();
					mainApp.runOnGLThread(new Runnable() {
						@Override
						public void run() {
							if (threadId == Thread.currentThread().getId())
								ParaEngineLuaJavaBridge.releaseLuaFunction(id);
						}
					});
				}
			}
		};
	}

	public LuaFunction(int id, Runnable runable)
	{
		m_id = id;

		m_threadId = Thread.currentThread().getId();

		m_releaseRunable = runable;
	}

	public int callWithString(String value)
	{
		if (m_id == -1 || m_threadId != Thread.currentThread().getId())
			return -1;
		return ParaEngineLuaJavaBridge.callLuaFunctionWithString(m_id, value);
	}
	
	public int release()
	{
		if (m_id == -1 || m_threadId != Thread.currentThread().getId())
			return -1;
		int id = m_id;
		m_id = -1;
		m_releaseRunable = null;
		return ParaEngineLuaJavaBridge.releaseLuaFunction(id);
	}

	@Override
	protected void finalize() throws Throwable
	{
		super.finalize();
		if (m_releaseRunable != null)
			m_releaseRunable.run();
	}
}