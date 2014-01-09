#!/usr/bin/env ruby

require 'json'
require 'net/http'
require 'pathname'
require 'pp'
require 'optparse'
require 'uri'

class Coveralls
  def initialize(*args)
    @verbose = false

    OptionParser.new do |opts|
      opts.banner = "Useage: coveralls.rb [options]"
      opts.on('-v', '--[no-]verbose', 'Run verbosely') do |v|
        @verbose = v
      end
    end.parse!(args)

    @dirs = args.dup
  end

  def report_coverage
    post_report(generate_report)
  end

  private

  def generate_report
    # Generate gcov reports
    puts "Generating gcov reports" if verbose?
    @dirs.each do |dir|
      Dir["#{File.expand_path(dir)}/**/*.gcno"].each do |gcno|
        puts "  #{gcno}" if verbose?
        run_gcov("#{File.dirname(gcno)}/#{File.basename(gcno, '.gcno')}.o")
      end
    end

    # Convert gcov reports to coveralls report
    puts "Generating coveralls.io report" if verbose?
    repo_root = Pathname.new(gitroot)
    source_files = Dir["#{Dir.pwd}/*.gcov"].map do |gcov|
      puts "  #{gcov}" if verbose?
      report(gcov, repo_root)
    end

    report = {
        'service_name' => 'travis-ci',
        'service_job_id' => ENV['TRAVIS_JOB_ID'],
        'git' => {
            'head' => {
                'id' => gitlog('%H'),
                'author_name' => gitlog('%aN'),
                'author_email' => gitlog('%ae'),
                'committer_name' => gitlog('%cN'),
                'committer_email' => gitlog('%ce'),
                'message' => gitlog('%s'),
            },
            'branch' => ENV['TRAVIS_BRANCH'] || gitbranch,
            'remotes' => gitremotes
        },
        'run_at' => Time.now.utc.to_s
    }

    pp report if verbose?

    report['source_files'] = source_files

    JSON.dump(report)
  end

  def post_report(report)
    url = URI('https://coveralls.io/api/v1/jobs')
    req = Net::HTTP::Post.new(url)
    req.content_type = 'multipart/form-data; boundary=AgBfCeDdEcFbGa'

    multipart_data = "--AgBfCeDdEcFbGa\n"
    multipart_data << "Content-Disposition: form-data; name=\"json_file\"\r\n"
    multipart_data << "Content-Type: application/json\r\n"
    multipart_data << "\r\n"
    multipart_data << report
    multipart_data << "\r\n"
    multipart_data << "--AgBfCeDdEcFbGa--\r\n"

    req.body = multipart_data
    res = Net::HTTP.start(url.hostname, url.port, :use_ssl => url.scheme == 'https' ) do |http|
      http.request(req)
    end

    if verbose?
      puts "Coveralls.io reponded with HTTP #{res.code}"
      puts res.message
    end
  end

  def verbose?
    @verbose
  end

  def gitlog(fmt)
    `git --no-pager log -1 --pretty=format:#{fmt}`
  end

  def gitroot()
    `git rev-parse --show-toplevel`.strip
  end

  def gitbranch()
    `git rev-parse --abbrev-ref HEAD`.strip
  end

  def gitremotes()
    `git remote -v`.lines.find_all { |l| l.index '(fetch)' }.map { |l| { 'name' => l.split[0], 'url' => l.split[1] } }
  end

  def run_gcov(object_file)
    `gcov #{object_file}`
  end

  def report(gcov_file, root_dir)
    source_file = ''
    src = ''
    cov = []
    gcov_line = /([^\:]+)\:([^\:]+)\:(.*)/
    File.foreach(gcov_file) do |line|
      match = gcov_line.match(line)
      next unless match

      exec_count = match[1].strip
      line_no = match[2].strip.to_i
      source_line = match[3]
      if line_no == 0
        tag,info = source_line.split(':', 2)
        source_file = info.strip if tag == 'Source'
      else
        src << '\n' unless src.empty?
        src << source_line
        if exec_count.index('#') || exec_count.index('=')
          cov << nil
        else
          cov << exec_count.to_i
        end
      end
    end

    {
      'name' => Pathname.new(source_file).relative_path_from(root_dir).to_s,
      'source' => src,
      'coverage' => cov
    }
  end
end

Coveralls.new(*ARGV).report_coverage
