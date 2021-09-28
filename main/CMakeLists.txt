idf_component_register(SRCS "main.c" 
                            "user_https_ota.c"
                            "wifi_utils.c"
                            "dht.c"
                            
                    INCLUDE_DIRS "." "include"
                    # Embed the server root certificate into the final binary
                    EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem)
