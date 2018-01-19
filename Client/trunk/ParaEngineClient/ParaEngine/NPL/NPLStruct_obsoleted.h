#pragma once
namespace NPL
{
	/**
	* Neuron file is the primary file in NPL. They are always simulated (executed) from where 
	* they live. I.e. Stimuli may come from another runtime and processed in the neuron file
	* and finally several messages may route to other NPL runtime. 
	*
	* Neurons (also spelled neurones or called nerve cells) are the primary cells of 
	* the nervous system
	*/
	struct NeuronFile
	{
	public:
		/// the NPL runtime state for this neuron file.
		/// if the state is NULL, it is a remote neuron file
		CNPLScriptingState* m_pRuntimeState;

		/// the number of passes that this file has been inactive
		/// NPL will unload files that has been inactive for too long, just to save space.
		int m_nInactivePassCount;
	};

	/**
	* GliaFiles in NPL are always executed in the receiver environment and share the 
	* same global environment in which they are executed. They may not have activate() 
	* function like the neuron file.They act as local files, and can be activated 
	* by a special function called NPL.ActivateCopy(), which will make a copy of the glia
	* file to the destination NPL runtime and execute it from there as a local file. 
	*
	* In the human brain, glia are estimated to outnumber neurons by as much as 50 to 1.
	* The only notable differences between neurons and glia, by modern scrutiny, are the
	* ability to generate action potentials and the polarity of neurons, namely the axons 
	* and dendrites which glia lack.
	*/ 
	struct GliaFile
	{
	public:
		GliaFile(){}
		~GliaFile(){}
	public:
		/** get the NPL runtime state*/
		CNPLScriptingState* GetRuntimeState(){return m_pRuntimeState;};
		/// the NPL runtime state for all glia File in NPL
		static CNPLScriptingState* m_pRuntimeState;
	};
}