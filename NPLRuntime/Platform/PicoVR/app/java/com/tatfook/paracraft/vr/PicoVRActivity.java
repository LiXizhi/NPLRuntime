//-----------------------------------------------------------------------------
// PicoVRActivity.java
// Authors: big
// CreateDate: 2026.10.28
// Desc: ParaEngineActivity for PICO headsets. The GLSurfaceView only provides the
// GL thread and an OpenGL ES 3 context, the frames go to the OpenXR swapchain.
//-----------------------------------------------------------------------------

package com.tatfook.paracraft.vr;

import com.tatfook.paracraft.ParaEngineActivity;
import com.tatfook.paracraft.ParaEngineGLSurfaceView;

public class PicoVRActivity extends ParaEngineActivity {
    @Override
    protected ParaEngineGLSurfaceView onCreateView() {
        final ParaEngineGLSurfaceView view = super.onCreateView();

        // XR_KHR_opengl_es_enable needs an OpenGL ES 3 context. The default chooser picks an ES3 config for it.
        // must be called before setRenderer(), which happens after onCreateView().
        view.setEGLContextClientVersion(3);
        view.setEGLConfigChooser(8, 8, 8, 8, 16, 0);

        return view;
    }
}
