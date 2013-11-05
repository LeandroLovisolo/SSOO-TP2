if(typeof $ == 'undefined'
    || typeof Cell == 'undefined'
	|| typeof Codigo == 'undefined') {
    alert('Faltan librerias Board' + $ + Board);
} 

var Board = function(agame, size, boatsize, player) {
	this.game = agame;
	this.size = size;
	if(typeof(player) == "undefined" || !player) {
		this._init_boats(boatsize)
		this._init_state();
	} else {
		//loguear('Caso codigo enemigo')
		//Caso codigo enemigo.
		this.tiles = boatsize
		//Reemplazar el board actual con este.
		this.enemy_state(player)
	}
	this.createUI();
}



Board.prototype._init_boats = function(boatsize) {
	this.nextboatid = 1;
	this.lastboatsize = 0;
	size = this.size;
    this.boat_sizes = {}; 	// Cantidad de botes con ese tamaño
	this.tiles = new Array(size);		
    this.boats = new Array(size);		// Los id de cada bote por ubicacion;
	for(x=0;x<size; x++) {
        this.tiles[x] = new Array(size);
        this.boats[x] = new Array(size);
        for(y=0;y<size; y++) {
            this.tiles[x][y] = Codigo.Agua;
            this.boats[x][y] = Codigo.BoteVacio;	// Vacio
        }
    }

    var cursize = boatsize;
    boat_size = Math.min(5, boatsize);
    while (cursize > 0) {
		if(boat_size in this.boat_sizes)
			this.boat_sizes[boat_size]++;
		else
			this.boat_sizes[boat_size] = 1;
		cursize = cursize - boat_size;
		if(boat_size == 1) {
			boat_size = cursize
		} else {
			boat_size = Math.min(cursize, boat_size-1);
		}
			
    }
}

Board.prototype._init_state = function() {
	this.set_state(new BuildState(this))
	board = this;
	$('#my-board').on('click','#addBoat', function(e) {
		board.add_boat()
		e.preventDefault()
	});
	$('#my-board').on('click','#Setup', function(e) {
		board.setup();
		e.preventDefault()
	});
}

Board.prototype.set_state = function(state) {
	this.state = state;
	$('#my-board').off('click');
    $('#my-board').on('click', '#cboard td', function(e) {
		cell = new Cell($(e.target));
		if(cell.estado in state)
			state[cell.estado](cell)
	});
}



Board.prototype.createUI = function() {
	str = "<table id='cboard'>"
	for(x=0;x<this.size;x++){
		str += '<tr id="'+x+'">';
		for(y=0;y<this.size;y++){
			str += '<td id="'+y+'" class="'+this.tiles[x][y]+'">'+this.tiles[x][y]+'</td>';
		}
		str += "</tr>";
	}
	str += '</table>'
	$('#my-board').html(str);
	if ('build'  in this.state) {
		str += '<br /><a id="addBoat" href="#">Agregar Bote</a>'
		str += '<br /><a id="Setup" href="#">Listo!</a>'
		str += '<br/><span id="info"></span>'
		$('#my-board').html(str);
		this.updateUI();
	}
	
}

Board.prototype.get_cell = function(x,y) {
	return new Cell($('#my-board #'+x+' #'+y));
}

Board.prototype.updateUI = function() {
	str = '<ul>'
	for (boat_size in this.boat_sizes) {
		str += '<li>' + this.boat_sizes[boat_size] + ' de ' + boat_size + ' casillas </li>' 
	}
	str += '</ul>';
	$('#my-board #info').html(str);
};


 Board.prototype.add_boat = function() {
	var myboard = this;
	if (!(this.lastboatsize in this.boat_sizes) || this.boat_sizes[this.lastboatsize] == 0) {
		warning("No se puede agregar un bote de este tamanio: " + this.lastboatsize );
	} else {
		if (this.lastboatsize != 0) {
			for (x=0;x<this.size;x++) {
				for (y=0;y<this.size;y++) {
					if (this.boats[x][y] == this.nextboatid) {
						cell = this.get_cell(x,y)
						cell.poner_bote(this.nextboatid)
					} 
				}
			}
			this.boat_sizes[this.lastboatsize]--;
			this.lastboatsize = 0;
			this.nextboatid++;
			loguear("Bote agregado");
		} else {
			warning("No se puede agregar un bote de tamanio 0");
		}
		this.updateUI("#my-board");
	}
}

Board.prototype.setup = function() {
	var myboard = this;
	faltan = false
	//Chequear que se hayan puesto los botes
	for (boat_size in this.boat_sizes) {
		if (myboard.boat_sizes[boat_size] != 0) {
			faltan = true;
			warning("Faltan " + myboard.boat_sizes[boat_size] + " botes de tamanio " + boat_size);
		}
	}
	if(faltan)
			return;
	//Construir el array de botes a enviar
	var boats = new Array(myboard.nextboatid-1);
	for (i=0;i<myboard.nextboatid-1;i++) {
		boats[i] = new Array();
	}
	for (x=0;x<myboard.size;x++) {
		for (y=0;y<myboard.size;y++) {
			if (myboard.boats[x][y] > 0) {
				boats[(myboard.boats[x][y]-1)].push({'x':x,'y':y});
			}
		}
	}
	this.game.events.set_player_board(boats);
	this.game.setup(boats);
}


Board.prototype.start = function() {
	this.set_state(new MyBoardState(this));
	$('#Setup,#info,#addBoat').hide()
}

Board.prototype.enemy_state = function(player) {
	this.set_state(new EnemyPlayerState(player));
}
