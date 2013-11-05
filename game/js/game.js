//Validar que haya jquery.
//Estructuras de comunicacion.
//Actualizaciones y eventos del juego (vista/control).
//Comunicación con el server. 
//Logica del juego
if(typeof $ == 'undefined'
    || typeof Board == 'undefined') {
    alert('Faltan librerias' + $ + Board);
} 

//Estado del juego.
var Game = function(connection) {
    this.connection = connection;
    this.id = null;
    this.size = null;
    this.current_board = null;
    this.target_board = null;
    this.my_board = null;
    this.players = null;
    this.events = null;
    this._register_callbacks();
    
}

Game.prototype._register_callbacks = function() {
    //Manejo de mensajes
    this.connection.add_callback('Start', $.proxy(this.do_start, this));
    this.connection.add_callback('Setup', $.proxy(this.do_setup, this));
    this.connection.add_callback('Subscribe', $.proxy(this.do_subscribe, this));
    this.connection.add_callback('Player_Info', $.proxy(this.do_get_info, this));
    this.connection.add_callback('Get_Update', $.proxy(this.do_get_update, this));
    this.connection.add_callback('Accept', $.proxy(this.do_wait_eta, this));
    this.connection.add_callback('Decline', $.proxy(this.do_decline, this));
    this.connection.add_callback('Event', $.proxy(this.do_process_event, this));
    this.connection.add_callback('Finish', $.proxy(this.do_finish, this));
}

Game.prototype.subscribe = function(name) {
    if (this.id != null) {
        error('Ya estás subscripto con el id ' + this.id);
    } else {
        this.connection.subscribe(name);
    }
}

Game.prototype.unsubscribe = function(name) {
    if (this.id == null) {
        error('No estás subscripto');
    } else {
        this.connection.unsubscribe(this.id);
        this.id = null;
    }
}

Game.prototype.setup = function(boats) {
    if (this.id == null) {
        error('No estas subscripto');
    } else {
        this.connection.setup(this.id, boats);
    }
}




Game.prototype.do_subscribe = function(payload) {
    if (this.id != null) {
        error('Ya estás jugando... mensaje "Subscribe" incorrecto');
    } else {
        this.id = payload['Id'];
        this.size = payload['Boardsize'];
        this.my_board = new Board(this, this.size, payload['Boatssize']);
        this.current_board = this.my_board;
        this.events = new Events(this.id, this.size)
    }
}

Game.prototype.do_start = function(players) {
    this.players = Array();
    for (x=0; x<players.length; x++) {
        id = players[x]['Id']
        name = players[x]['Name']
        this.players[id] = new Player(id, name);
    }
    
    $('#l-jugadores').on('click','a', function(e) {
        id = $(e.target).attr('id')
        game.players[id].update_board()
        $('#l-jugadores li').removeClass('selected');
        $('#l-jugadores li a').removeClass('selected');
        $(e.target).parent().addClass('selected');
    });
    
    this.events.draw_player_board();
    paneles.overlay_eta.hide();
}

Game.prototype.do_get_update = function(payload) {
    this.events.new_event(payload)
    paneles.overlay_eta.hide();
}

Game.prototype.do_setup = function(payload) {
    loguear("Setup: " + payload);
    this.my_board.start()
    this.wait_for_start()
}

Game.prototype.wait_for_start = function() {
    paneles.eta[0].innerHTML = "Esperando que el resto de los jugadores finalize con la ubicación de los barcos";
    paneles.overlay_eta.show();
}

Game.prototype.do_get_info = function(payload) {
    json_board = payload['board']
    player_id = payload['t_id']
    current_player = this.players[player_id]
    $('#l-jugadores #'+player_id).addClass('selected')
    this.current_board = new Board(this, this.size, json_board, current_player)
    current_player.set_board(this.current_board)
}

Game.prototype.do_wait_eta = function(payload) {
    eta = payload['eta'];
    paneles.eta[0].innerHTML = "ETA<br />"+eta + " ms";
    paneles.overlay_eta.show();
    eta = eta - 25;
    payload['eta'] = eta;
    if (eta > 0) {
        setTimeout($.proxy(this.do_wait_eta,this),25, payload);
    } else {
        paneles.eta[0].innerHTML = "ETA<br /> 0 ms <br /> <strong>Shoot</strong>";
        setTimeout($.proxy(this.connection.get_update,this.connection),100) //corrección de tiempos, les doy 100ms mas de changui        
    }
}

Game.prototype.do_decline = function(payload) {
    msg = payload['Msg'];
    paneles.eta[0].innerHTML = "Denegado<br />"+msg + "";
    paneles.overlay_eta.show();
    setTimeout($.proxy(this.do_finish_decline, this),1000);
}

Game.prototype.do_finish_decline = function() {
    paneles.overlay_eta.hide();
}


Game.prototype.do_finish = function(payload) {
    this.connection.ack(this.id);
    paneles.eta[0].innerHTML = "Juego Finalizado";
    paneles.overlay_eta.show();
    setTimeout($.proxy(this.do_finish_finish, this),2000);
}

Game.prototype.do_finish_finish = function() {
    paneles.overlay_eta.hide();
}

Game.prototype.do_process_event = function(payload) {
    t_id = payload['t_id'];
    s_id = payload['s_id'];
    if (this.events != null) {
        this.events.new_event(payload);
    }
    if (s_id == game.id) {
        paneles.overlay_eta.hide();
        this.connection.get_info(t_id);
    }
}
