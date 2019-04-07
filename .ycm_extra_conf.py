def Settings( **kwargs ):
    return {
	    'flags': [
		'-x', 'c',
		'-Wall',
		'-Wextra',
		'-Werror',
		'-Isrc',
                '-Ires',
		'-Iinc',
		'-I../SGDK/inc',
		'-I../SGDK/res',
		'-Itests/cmocka/include',
                '-Itests' ]
	    }
