
class UserInterface
  constructor: ->
    $('form').submit @onConsoleSubmit
    
  onConsoleSubmit: (e) ->
    e.preventDefault()
    c.ws.send $('form input:first').val()

    # JSON.stringify({ 
    #   command : 
    # })
    
class Renderer
  constructor: ->
    SCREEN_WIDTH = window.innerWidth
    SCREEN_HEIGHT = window.innerHeight;	

    VIEW_ANGLE = 45
    ASPECT = SCREEN_WIDTH / SCREEN_HEIGHT
    NEAR = 0.1
    FAR = 20000

    @scene = new THREE.Scene()
    @camera = new THREE.PerspectiveCamera( VIEW_ANGLE, ASPECT, NEAR, FAR)
    @scene.add(@camera)

    @camera.position.set(0,150,150)
    @camera.lookAt(@scene.position)

    light = new THREE.SpotLight(0xffffff)
    light.position.set(100,200,300)
    light.target.position.set(0, 0, 0)
    # light.shadowCameraVisible = true
    light.shadowDarkness = 0.25
    light.intensity = 2
    light.castShadow = true
    @scene.add(light)
    
    # light.shadowCameraNear = 2;
    # light.shadowCameraFar = 5;
    # light.shadowCameraLeft = -0.5;
    # light.shadowCameraRight = 0.5;
    # light.shadowCameraTop = 0.5;
    # light.shadowCameraBottom = -0.5;

    ambientLight = new THREE.AmbientLight(0x111111)
    @scene.add(ambientLight)

    groundMaterial = new THREE.MeshBasicMaterial({ color: 0x003388 })
    plane = new THREE.Mesh(new THREE.PlaneGeometry(1000, 1000), groundMaterial)
    plane.rotation.x = -Math.PI / 2
    plane.position.y = 3
    plane.receiveShadow = true
    @scene.add(plane)

    boxgeometry = new THREE.CubeGeometry(5, 5, 5);
    boxmaterial = new THREE.MeshLambertMaterial({ color: 0xCCCCCC })

    @cube = new THREE.Mesh(boxgeometry, boxmaterial)
    @cube.castShadow = true
    @cube.position.x = 0
    @cube.position.y = 10
    @cube.position.z = 0
    # @cube.useQuaternion = true
    @scene.add(@cube)

    loader = new THREE.ColladaLoader()
    loader.load './models/cheetah.dae',  (result) =>
      console.log result
      @model = result.scene
      @model.scale.x = @model.scale.y = @model.scale.z = 2
      # @model.material.overdraw = true
      # @scene.add(@model)

    @webglRenderer = new THREE.WebGLRenderer()
    @webglRenderer.setSize(SCREEN_WIDTH, SCREEN_HEIGHT)
    @webglRenderer.domElement.style.position = "relative"
    @webglRenderer.shadowMapEnabled = true
    @webglRenderer.shadowMapSoft = true

    container = document.createElement('div')
    document.body.appendChild(container)
    container.appendChild(@webglRenderer.domElement)
    # window.addEventListener('resize', onWindowResize, false)

  animate: =>
    timer = Date.now() * 0.002
    
    if @model
      @model.rotation.x = -Math.PI / 2
      @model.rotation.z += 0.1
    
    @camera.position.x = Math.cos(timer) * 50
    @camera.position.z = Math.sin(timer) * 50
    @camera.position.y = 50
    @camera.lookAt(@scene.position)
    
    # cube.rotation.y += 0.05;
    requestAnimationFrame(@animate)
    @render()
  
  render: =>
    @camera.lookAt(@scene.position)
    @webglRenderer.render(@scene, @camera)


class Connection
  constructor: ->
    console.log "Connecting..."
    
    @ws = new WebSocket("ws://127.0.0.1:7681/", "default");
    @ws.onopen = =>
      console.log "Opened!"
      @ws.send "Hello World!"
      
    @ws.onclose = =>
      console.log "Closed!"
      
    @ws.onmessage = @onMessage
    
  onMessage: (e) =>
    # that.messageCountStats.accumulate(1);
    # messageCount++;

    if e.data && e.data instanceof Blob
      # console.log "Got a blob..."

      fr = new FileReader()
      fr.onload = (f) ->
        # console.log fr.result.byteLength

        ary = new Int32Array(fr.result)
        # console.log "id: " + ary[0].toString(16)

        f = 1.0
        
        ary = new Float32Array(fr.result)
        r.cube.position = new THREE.Vector3(ary[1], ary[2], ary[3])
        r.cube.rotation = new THREE.Vector3(ary[4], ary[5], ary[6])
        # console.log r.cube.rotation
        # r.cube.quaternion = new THREE.Quaternion(ary[4] / f, ary[5] / f, ary[6] / f, ary[7] / f)
        # r.cube.quaternion.normalize()
        
        # console.log(position)
        # console.log(rotation)
        
        # that.downStats.accumulate(fr.result.byteLength);
        # updateWorldFromBuffer(fr.result);
        # that.dispatchEvent(updateEvent);

      fr.readAsArrayBuffer(e.data)

      # ...
      #       `
      # // Update transforms. Need a filereader to read the fetched binary blob
    else if typeof e.data == "string"
      console.log "Got a string..." + e.data
      # // We got a world description in JSON. Parse it.
      # try {
      # var world = JSON.parse(e.data/*.match(/\[.*\]/)*/);
      # } catch(err){
      # console.log(e.data);
      # throw err;
      # }
      # that.updateWorldFromJSON(world);
      # that.dispatchEvent(changeEvent);
      # }
      # };
    
    
    
window.c = new Connection
window.r = new Renderer
window.ui = new UserInterface

r.animate();
