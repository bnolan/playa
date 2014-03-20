
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

    @objects = {}
    
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

    groundMaterial = new THREE.MeshBasicMaterial({ color: 0xaaaaaa })
    plane = new THREE.Mesh(new THREE.PlaneGeometry(2000, 2000), groundMaterial)
    plane.rotation.x = -Math.PI / 2
    plane.position.y = -1
    plane.receiveShadow = true
    @scene.add(plane)

    loader = new THREE.ColladaLoader()
    loader.load './models/tree.dae',  (result) =>
      # otherwise castshadow doesn't work... :/
      model = result.scene.children[0].children[0]
      model.scale.x = model.scale.y = model.scale.z = 10
      model.castShadow = true
      model.receiveShadow = false
      @scene.add(model)

    @avatar = new Skin(THREE, '/skins/slenderman.png')
    @avatar.mesh.position.y = 0
    @avatar.mesh.position.z = 10
    @avatar.mesh.scale.x = @avatar.mesh.scale.y = @avatar.mesh.scale.z = 0.5
    # @avatar.mesh.castShadow = true
    @scene.add(@avatar.mesh)

    @webglRenderer = new THREE.WebGLRenderer()
    @webglRenderer.setSize(SCREEN_WIDTH, SCREEN_HEIGHT)
    @webglRenderer.domElement.style.position = "relative"
    @webglRenderer.shadowMapEnabled = true
    @webglRenderer.shadowMapSoft = true

    container = document.createElement('div')
    document.body.appendChild(container)
    container.appendChild(@webglRenderer.domElement)
    # window.addEventListener('resize', onWindowResize, false)
    
    @addStats()
    
  addStats: ->
    @stats = new Stats();
    @stats.setMode(0) # 0: fps, 1: ms

    # Align top-left
    @stats.domElement.style.position = 'absolute';
    @stats.domElement.style.left = '0px';
    @stats.domElement.style.top = '0px';

    document.body.appendChild(@stats.domElement)

  addObject: (id) ->
    boxgeometry = new THREE.CubeGeometry(10, 10, 10);
    boxmaterial = new THREE.MeshLambertMaterial({ color: 0xAAAAAA })

    cube = new THREE.Mesh(boxgeometry, boxmaterial)
    cube.castShadow = true
    cube.position.x = 0
    cube.position.y = 10
    cube.position.z = 0
    # cube.useQuaternion = true
    @scene.add(cube)
    
    @objects[id] = cube
    
    cube

  removeObject: (object) ->
    @scene.remove(object)
    delete @objects[object.id]
    
  hasObject: (id) ->
    !!@objects[id]
    
  animate: =>
    timer = Date.now() * 0.0005
    
    # if @model
    #   @model.rotation.x = -Math.PI / 2
    #   @model.rotation.z += 0.1
    
    # @camera.position.x = Math.cos(timer) * 100
    # @camera.position.z = Math.sin(timer) * 100
    # @camera.position.y = 100
    # @camera.lookAt(@scene.position)
    
    # cube.rotation.y += 0.05;
    requestAnimationFrame(@animate)

    @stats.begin()
    @render()
    @stats.end()
  
  render: =>
    # @camera.lookAt(@scene.position)
    @webglRenderer.render(@scene, @camera)


class Connection
  constructor: ->
    console.log "Connecting..."
    
    host = window.location.host.split(":")[0]
    
    @stopProcessing = false

    @ws = new WebSocket("ws://#{host}:7681/", "default");
    
    @ws.binaryType = 'arraybuffer'
    
    @ws.onopen = =>
      console.log "Opened!"
      @ws.send "Hello World!"
      
    @ws.onclose = =>
      console.log "Closed!"
      
    @ws.onmessage = @onMessage
  
  toHex: (i) ->
    r = parseInt(i).toString(16)
    if r.length == 1
      r = "0#{r}"
    r
    
  onMessage: (e) =>
    # that.messageCountStats.accumulate(1);
    # messageCount++;

    if @stopProcessing
      return

    littleE = true # little endian?
    
    if e.data && e.data instanceof ArrayBuffer
      # console.log e.data
      view = new DataView(e.data, 0, e.data.length)

      id = view.getUint32(0, littleE).toString(16)
      type = view.getUint8(4)

      obj = if r.hasObject(id)
        r.objects[id]
      else
        r.addObject(id)
        
      if type == 0x20
        r.removeObject(obj)
      else if type = 0x10
        # ary = new Float32Array(e.data.slice(8,32))
        # obj.position = new THREE.Vector3(ary[0], ary[1], ary[2])
        obj.position = new THREE.Vector3(view.getFloat32(8, littleE), view.getFloat32(12, littleE), view.getFloat32(16, littleE))
        # obj.quaternion = new THREE.Quaternion(ary[4] / f, ary[5] / f, ary[6] / f, ary[7] / f)
      
      
      
    else if e.data && e.data instanceof Blob
      # console.log "Got a blob..."

      fr = new FileReader()
      fr.onload = (f) =>
        # console.log fr.result.byteLength
          
        ary = new UInt32Array(fr.result)
        id = ary[0].toString(16)

        f = 1.0
        
        if r.hasObject(id)
          obj = r.objects[id]
        else
          console.log "adding object..."
          obj = r.addObject(id)
          
        ary = new Float32Array(fr.result)
        obj.position = new THREE.Vector3(ary[1], ary[2], ary[3])
        # r.cube.rotation = new THREE.Vector3(ary[4], ary[5], ary[6])
        # console.log r.cube.rotation
        obj.quaternion = new THREE.Quaternion(ary[4] / f, ary[5] / f, ary[6] / f, ary[7] / f)
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
    
    
class Game
  constructor: (renderer) ->
    @keyboard = new KeyboardState
    @renderer = renderer

    element = document.body

    document.addEventListener( 'pointerlockchange', @pointerlockchange, false )
    document.addEventListener( 'mozpointerlockchange', @pointerlockchange, false )
    document.addEventListener( 'webkitpointerlockchange', @pointerlockchange, false )

    element.requestPointerLock = element.requestPointerLock || element.mozRequestPointerLock || element.webkitRequestPointerLock
    element.requestPointerLock()

    controls = new THREE.PointerLockControls(@renderer.camera)
    @renderer.scene.add(controls.getObject())
    
    setInterval(@tick, 1000/20)

  pointerlockchange: (event) ->
    # if (document.pointerLockElement === element || document.mozPointerLockElement === element || document.webkitPointerLockElement === element)
      # controls.enabled = true;
      # blocker.style.display = 'none';
    # else
      # controls.enabled = false
      # blocker.style.display = '-webkit-box';
      # blocker.style.display = '-moz-box';
      # blocker.style.display = 'box';
      # instructions.style.display = '';
    
  tick: =>
    d = 4.0

    @keyboard.update();
    
    if ( @keyboard.pressed("A") )
      @renderer.avatar.mesh.translateX(-d)

    if ( @keyboard.pressed("D") )
      @renderer.avatar.mesh.translateX(d)
    
    if ( @keyboard.pressed("W") )
      @renderer.avatar.mesh.translateZ(d)

    if ( @keyboard.pressed("S") )
      @renderer.avatar.mesh.translateZ(-d)

    @renderer.camera.position = @renderer.avatar.mesh.position.clone()
    @renderer.camera.translateY(20) # .set(0,150,150)
    @renderer.camera.lookAt(new THREE.Vector3(0,15,0)) # r.scene.position.clone().translateY(20))
    
window.r = new Renderer
window.c = new Connection
window.g = new Game(r)
window.ui = new UserInterface
# window.clock = new THREE.Clock


r.animate();
