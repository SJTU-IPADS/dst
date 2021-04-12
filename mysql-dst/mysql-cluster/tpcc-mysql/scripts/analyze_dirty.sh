cat $1 | awk ' BEGIN { dt=-1;dr=-1;df=-1 } /Innodb_buffer_pool_pages_data/ { if (dt==-1) { dt=$4} else { dt+=$4  } }  /Innodb_buffer_pool_pages_dirty/ { if (dr==-1) { dr=$4} else { dr+=$4  } } /Innodb_buffer_pool_pages_free/ { if (df==-1) { df=$4} else { df+=$4  } ; print (100*dr)/(1+dt+df)  } '
 