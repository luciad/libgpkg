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
    source_files = []
    @dirs.each do |dir|
      Dir["#{File.expand_path(dir)}/**/*.gcov"].each do |gcov|
        puts "  #{gcov}" if verbose?
        source_files << report(gcov, repo_root)
      end
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

    puts JSON.pretty_generate(report) if verbose?

    report['source_files'] = source_files

    report_path = File.expand_path('report.json')
    File.open(report_path, 'w') { |f| JSON.dump( report, f ) }
    report_path
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
        src << "\n" unless src.empty?
        src << source_line
        if exec_count.index('-')
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

  def post_report(report)
    system('curl', "-Fjson_file=@#{report}", 'https://coveralls.io/api/v1/jobs')
  end

  def verbose?
    @verbose
  end

  def debug?
    true
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

  def run_gcov(file_name)
    system('gcov', File.basename(file_name), :chdir => File.dirname(file_name))
  end
end

Coveralls.new(*ARGV).report_coverage
