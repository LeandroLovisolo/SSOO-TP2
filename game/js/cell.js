var Cell = function(element) {
    this.element = element;
    this.x = element.parent().attr('id');
    this.y = element.attr('id');
    this.estado = element.attr('class')
    this.texto = element.html();
}



Cell.prototype._update = function() {
    this.element.attr('class',this.estado);
    this.element.html(this.texto);
}

Cell.prototype.poner_bote_entrando = function(boatid) {
    this.estado = Codigo.BoteEntrando
    this.texto = boatid
    this._update()
}

Cell.prototype.poner_bote = function(boatid) {
    this.estado = Codigo.Bote
    this.texto = boatid
    this._update()
}

Cell.prototype.poner_agua = function() {
    this.estado = Codigo.Agua
    this.texto = Codigo.Agua
    this._update()
}

Cell.prototype.cambiar_texto = function(texto) {
    this.texto = texto
    this._update() 
}


Cell.prototype.get_coord = function() {
    return {'x':this.x+'', 'y':this.y+''}
}
