all: BSD/cgi_id.exe

#include "tmpdep"

OBJS  = BSD/db_bugbase.o               \
        BSD/bugrep_actions.o        \
	BSD/cgi_id.o                \
	BSD/page_doc_request.o \
        BSD/cgic.o		    \
	BSD/connection_zone.o       \
        BSD/file_io_subsystem.o     \
        BSD/html_subsystem.o        \
        BSD/message_subsystem.o     \
        BSD/net_subsystem.o         \
        BSD/page_add_bug_result.o   \
        BSD/page_add_user.o         \
	BSD/users.o			\
	BSD/rank_calc.o			\
	BSD/page_mass_user_options.o \
        BSD/page_add_user_result.o  \
        BSD/page_daemon_stats.o     \
        BSD/page_delete_user.o      \
        BSD/page_edit_bug.o         \
        BSD/page_edit_user.o        \
        BSD/page_edit_user_result.o \
        BSD/page_elements.o         \
        BSD/page_find_bug.o         \
        BSD/page_list_bugs.o        \
        BSD/page_user_listing.o     \
        BSD/page_web_stats.o        \
        BSD/page_main_menu.o        \
        BSD/page_authenticate.o     \
        BSD/page_bug_repository.o   \
        BSD/page_add_bug.o          \
	BSD/page_sysset.o           \
	BSD/page_download.o         \
	BSD/page_download_reg.o     \
	BSD/page_homepage.o         \
	BSD/page_invalid_request.o  \
	BSD/page_news.o             \
	BSD/page_faq.o              \
	BSD/page_order.o	    \
	BSD/page_news_admin.o       \
        BSD/page_faq_admin.o        \
	BSD/page_ranking.o          \
	BSD/page_support.o          \
        BSD/port_subsystem.o        \
        BSD/srvstat_actions.o       \
	BSD/db_system_settings.o    \
	BSD/db_news_articles.o	    \
	BSD/db_faq_entries.o	    \
	BSD/database_subsystem.o    \
	BSD/license_manager.o       \
	BSD/mime64.o                \
	BSD/mail_encode.o

CC     = gcc
LIBS   = 
CFLAGS = -DUNIX

BSD/users.o: users.c
	$(CC) $(CFLAGS) -c -o BSD/users.o users.c

BSD/rank_calc.o: rank_calc.c
	$(CC) $(CFLAGS) -c -o BSD/rank_calc.o rank_calc.c

BSD/db_bugbase.o: db_bugbase.c
	$(CC) $(CFLAGS) -c -o BSD/db_bugbase.o db_bugbase.c

BSD/db_news_articles.o: db_news_articles.c
	$(CC) $(CFLAGS) -c -o BSD/db_news_articles.o db_news_articles.c

BSD/db_faq_entries.o: db_faq_entries.c
	$(CC) $(CFLAGS) -c -o BSD/db_faq_entries.o db_faq_entries.c

BSD/bugrep_actions.o: bugrep_actions.c
	$(CC) $(CFLAGS) -c -o BSD/bugrep_actions.o bugrep_actions.c

BSD/cgi_id.o: cgi_id.c
	$(CC) $(CFLAGS) -c -o BSD/cgi_id.o cgi_id.c

BSD/cgic.o: cgic.c
	$(CC) $(CFLAGS) -c -o BSD/cgic.o cgic.c

BSD/connection_zone.o: connection_zone.c
	$(CC) $(CFLAGS) -c -o BSD/connection_zone.o connection_zone.c

BSD/file_io_subsystem.o: file_io_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/file_io_subsystem.o file_io_subsystem.c

BSD/html_subsystem.o: html_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/html_subsystem.o html_subsystem.c

BSD/message_subsystem.o: message_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/message_subsystem.o message_subsystem.c

BSD/net_subsystem.o: net_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/net_subsystem.o net_subsystem.c



BSD/page_doc_request.o: page_doc_request.c
	$(CC) $(CFLAGS) -c -o BSD/page_doc_request.o page_doc_request.c

BSD/page_add_bug_result.o: page_add_bug_result.c
	$(CC) $(CFLAGS) -c -o BSD/page_add_bug_result.o page_add_bug_result.c

BSD/page_add_user.o: page_add_user.c
	$(CC) $(CFLAGS) -c -o BSD/page_add_user.o page_add_user.c

BSD/page_mass_user_options.o: page_mass_user_options.c
	$(CC) $(CFLAGS) -c -o BSD/page_mass_user_options.o page_mass_user_options.c

BSD/page_add_user_result.o: page_add_user_result.c
	$(CC) $(CFLAGS) -c -o BSD/page_add_user_result.o page_add_user_result.c

BSD/page_daemon_stats.o: page_daemon_stats.c
	$(CC) $(CFLAGS) -c -o BSD/page_daemon_stats.o page_daemon_stats.c

BSD/page_delete_user.o: page_delete_user.c
	$(CC) $(CFLAGS) -c -o BSD/page_delete_user.o page_delete_user.c

BSD/page_edit_bug.o: page_edit_bug.c
	$(CC) $(CFLAGS) -c -o BSD/page_edit_bug.o page_edit_bug.c

BSD/page_edit_user.o: page_edit_user.c
	$(CC) $(CFLAGS) -c -o BSD/page_edit_user.o page_edit_user.c

BSD/page_edit_user_result.o: page_edit_user_result.c
	$(CC) $(CFLAGS) -c -o BSD/page_edit_user_result.o page_edit_user_result.c

BSD/page_elements.o: page_elements.c
	$(CC) $(CFLAGS) -c -o BSD/page_elements.o page_elements.c

BSD/page_find_bug.o: page_find_bug.c
	$(CC) $(CFLAGS) -c -o BSD/page_find_bug.o page_find_bug.c

BSD/page_list_bugs.o: page_list_bugs.c
	$(CC) $(CFLAGS) -c -o BSD/page_list_bugs.o page_list_bugs.c

BSD/page_user_listing.o: page_user_listing.c
	$(CC) $(CFLAGS) -c -o BSD/page_user_listing.o page_user_listing.c

BSD/page_web_stats.o: page_web_stats.c
	$(CC) $(CFLAGS) -c -o BSD/page_web_stats.o page_web_stats.c

BSD/port_subsystem.o: port_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/port_subsystem.o port_subsystem.c

BSD/srvstat_actions.o: srvstat_actions.c
	$(CC) $(CFLAGS) -c -o BSD/srvstat_actions.o srvstat_actions.c

BSD/page_main_menu.o: page_main_menu.c
	$(CC) $(CFLAGS) -c -o BSD/page_main_menu.o page_main_menu.c

BSD/page_authenticate.o: page_authenticate.c
	$(CC) $(CFLAGS) -c -o BSD/page_authenticate.o page_authenticate.c

BSD/page_bug_repository.o: page_bug_repository.c
	$(CC) $(CFLAGS) -c -o BSD/page_bug_repository.o page_bug_repository.c

BSD/page_add_bug.o: page_add_bug.c
	$(CC) $(CFLAGS) -c -o BSD/page_add_bug.o page_add_bug.c

BSD/page_order.o: page_order.c
	$(CC) $(CFLAGS) -c -o BSD/page_order.o page_order.c

BSD/page_sysset.o: page_sysset.c
	$(CC) $(CFLAGS) -c -o BSD/page_sysset.o page_sysset.c

BSD/db_system_settings.o: db_system_settings.c
	$(CC) $(CFLAGS) -c -o BSD/db_system_settings.o db_system_settings.c

BSD/database_subsystem.o: database_subsystem.c
	$(CC) $(CFLAGS) -c -o BSD/database_subsystem.o database_subsystem.c

BSD/page_download.o: page_download.c
	$(CC) $(CFLAGS) -c -o BSD/page_download.o page_download.c

BSD/page_download_reg.o: page_download_reg.c
	$(CC) $(CFLAGS) -c -o BSD/page_download_reg.o page_download_reg.c

BSD/page_homepage.o: page_homepage.c
	$(CC) $(CFLAGS) -c -o BSD/page_homepage.o page_homepage.c

BSD/page_invalid_request.o: page_invalid_request.c
	$(CC) $(CFLAGS) -c -o BSD/page_invalid_request.o page_invalid_request.c

BSD/page_news.o: page_news.c
	$(CC) $(CFLAGS) -c -o BSD/page_news.o page_news.c

BSD/page_faq.o: page_faq.c
	$(CC) $(CFLAGS) -c -o BSD/page_faq.o page_faq.c

BSD/page_news_admin.o: page_news_admin.c
	$(CC) $(CFLAGS) -c -o BSD/page_news_admin.o page_news_admin.c

BSD/page_faq_admin.o: page_faq_admin.c
	$(CC) $(CFLAGS) -c -o BSD/page_faq_admin.o page_faq_admin.c

BSD/page_ranking.o: page_ranking.c
	$(CC) $(CFLAGS) -c -o BSD/page_ranking.o page_ranking.c

BSD/page_support.o: page_support.c
	$(CC) $(CFLAGS) -c -o BSD/page_support.o page_support.c

BSD/license_manager.o: license_manager.c
	$(CC) $(CFLAGS) -c -o BSD/license_manager.o license_manager.c

BSD/mime64.o:mime64.c
	$(CC) $(CFLAGS) -c -o BSD/mime64.o mime64.c

BSD/mail_encode.o: mail_encode.c
	$(CC) $(CFLAGS) -c -o BSD/mail_encode.o mail_encode.c

BSD/cgi_id.exe: $(OBJS) dep
	$(CC) -o BSD/cgi_id.exe $(OBJS) $(LIBS)
	#cp BSD/cgi_id.exe /usr/home/edenstud/www/cgi-bin/cgi_id.exe

depend dep:
	@for i in *.c;do echo -n;$(CC) -M $$i;done > tmpdep

strip:
	@for i in *.h;do (cat $$i | tr -d "\r" > tmp); mv -f tmp $$i;done
	@for i in *.c;do (cat $$i | tr -d "\r" > tmp); mv -f tmp $$i;done

	@rm -f tmp

clean:
	@echo cleaning intermediate build files...
	@rm BSD/*.o
	@echo done cleaning!




