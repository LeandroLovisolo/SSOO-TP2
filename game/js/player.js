var Player = function(id, name) {
    this.id = id
    this.name = name
    this.createUI();
}

Player.prototype.createUI = function() {
    j = '<li><a href="#"  id="'+this.id+'">'+this.name+'</a></li>'
    $('#l-jugadores').append(j)
    j = this
}

Player.prototype.update_board = function() {
    game.connection.get_info(this.id);
}

Player.prototype.set_board = function(board) {
    this.board = board
}
