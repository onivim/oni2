find extensions -type f -name '*.js.map' -exec rm "{}" +
find extensions -type f -name '*.ts' ! -path '*/node_modules/*' -exec rm "{}" + 
