
SUBDIRS = sthread_lib snfs_server snfs_lib 

#
# Dados sobre o grupo
# GRUPO = indicar o numero do grupo
# ALUNO1/ALUNO2/ALUNO3 = indicar os nomes dos estudantes (primeiro.último)
#
GRUPO=
ALUNO1=
ALUNO2=
ALUNO3=


all: build

build:
	@list='$(SUBDIRS)'; for p in $$list; do \
	  echo "Building $$p"; \
	  $(MAKE) -C $$p; \
	done

clean:
	@list='$(SUBDIRS)'; for p in $$list; do \
	  echo "Cleaning $$p"; \
	  $(MAKE) clean -C $$p; \
	done

package: clean zip

zip:
ifndef GRUPO
	@echo "ERROR: Must setup macro 'GRUPO' correcly."
else
	tar -czf project-$(GRUPO)-$(ALUNO1)-$(ALUNO2)-$(ALUNO3).tgz * 
endif
endif
endif
