
//State de construccion
var BuildState = function(board) {
    this.board = board;
    this['build'] = true
    this[Codigo.Agua] = this.marcar_bote;
    this[Codigo.BoteEntrando] = this.borrar_bote_actual;
    this[Codigo.Bote] = this.borrar_bote;
}


BuildState.prototype.marcar_bote = function(cell) {
    x = cell.x
    y = cell.y
	var myboard = this.board;
    if (myboard.lastboatsize == 5) {
        warning("Ya alcanzo el tamanio maximo permitido");
        return;
    }
    myboard.boats[x][y] = myboard.nextboatid;
    myboard.lastboatsize ++;
    cell.poner_bote_entrando(myboard.nextboatid);
}

BuildState.prototype.borrar_bote_actual = function(cell) {
    x = cell.x
    y = cell.y
    this.board.boats[x][y] = 0;
    this.board.lastboatsize--;
    cell.poner_agua();
}



BuildState.prototype.borrar_bote = function(cell) {
    id = cell.texto
    myboard = this.board
	if (confirm("¿Seguro que quiere eliminar el bote " + id + "?")) {
		var boatlen = 0;
		for (x=0;x<myboard.size;x++) {
			for (y=0;y<myboard.size;y++) {
                if (myboard.boats[x][y] != Codigo.BoteVacio) {
    				if (myboard.boats[x][y] > id) {
    					myboard.boats[x][y]--;
                        cell = myboard.get_cell(x,y)
                        cell.cambiar_texto(myboard.boats[x][y])
    				} else if (myboard.boats[x][y] == id) {
    					myboard.boats[x][y] = Codigo.BoteVacio;
    					boatlen++;
                        cell = this.board.get_cell(x,y);
                        cell.poner_agua();
    				} 
                }
			}
		}
		myboard.nextboatid--;
		myboard.boat_sizes[boatlen]++;
        myboard.updateUI();
		//myboard.updateUI("#my-board");
	}
}

//State de solo lectura.
var MyBoardState = function(board) {
    
}


var EnemyPlayerState = function(player) {
    this.player = player
    this[Codigo.Unknown] = function(cell) {
	game.connection.shoot(player.id, cell.get_coord());
    }
}
