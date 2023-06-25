assist - Build Assistance
===

These files "assist" when building with MakeMe, Gulp and working with GitLab.

* build-parts           - Invoke "me", "make" or "gulp" to build parts of an app
* cdrc                  - Copy to .cdrc to define environment for interactive use
* common                - Included by other scripts
* configure             - Core logic for top level configure script
* configure-node        - Configuration for node apps
* deploy-parts          - Invoke "me", "make" or "gulp" to deploy parts of an app
* docker-login          - Functions to login to the AWS ECR docker repository
* docker-publish        - Publish an image to the AWS ECR docker repository
* index.js              - Module to read configuration for gulp projects
* json2env              - Convert a JSON config file to environment key=value commands
* publish-parts         - Invoke "me", "make" or "gulp" to publish parts of an app
* remote-access         - Included script to configure remote access via SSH to AWS
* s3-publish-files      - Copy file tree to S3
