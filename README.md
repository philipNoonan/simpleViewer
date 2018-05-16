# simpleViewer

We need the following 3rd party libs installed.

<p>VTK (+ dependencies inc DICOMParser) https://www.vtk.org/download/ </p>
<p>glew http://glew.sourceforge.net/</p>
<p>glfw3 http://www.glfw.org/docs/latest/</p>
<p>glm https://glm.g-truc.net/0.9.8/index.html</p>
<p>imgui https://github.com/ocornut/imgui</p>

For a simple install, consider using vcpkg ( https://github.com/Microsoft/vcpkg ) The CMakeLists in the project assumes that vcpkg is used to manage 3rd party libraries. If you are using your own or building from source, you will have to link accordingly.

<p>To install vcpkg and install the required libs, use the following steps...</p>
<p>
<pre><code>&gt; git clone https://github.com/Microsoft/vcpkg
&gt; cd vcpkg
PS&gt; .\bootstrap-vcpkg.bat
Ubuntu:~/$ ./bootstrap-vcpkg.sh
</code></pre>
<p>Then, to hook up user-wide integration, run (note: requires admin on first use)</p>
<pre><code>PS&gt; .\vcpkg integrate install
Ubuntu:~/$ ./vcpkg integrate install
</code></pre>

<p>On windows, set the default install target to x64-windows by setting environment variables </p>
<pre><code>PS&gt;
setx VCPKG_DEFAULT_TRIPLET "x64-windows" /m
</code></pre>

<p>Install the required packages with </p>
<pre><code>PS&gt; .\vcpkg install vtk glew glfw3 glm imgui
Ubuntu:~/$ ./vcpkg install vtk glew glfw3 glm imgui
</code></pre>
